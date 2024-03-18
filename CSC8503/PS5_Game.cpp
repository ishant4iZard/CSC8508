#include "PS5_Game.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "GravityWell.h"
#include "NavigationGrid.h"
#include "PS5MenuSystem.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"

#include "../PS5Core/PS5Window.h"

#include <iostream>
#include <string>

using namespace NCL;
using namespace CSC8503;

NCL::CSC8503::PS5_Game::PS5_Game()
{
	appState = ApplicationState::GetInstance();
	ui = UIPlaystation::GetInstance();
	
	//StartLevel();

	Menu = new PS5MenuSystem(this);
	debugHUD = new DebugHUD();
	//audioEngine = new AudioEngine();
}

NCL::CSC8503::PS5_Game::~PS5_Game()
{
	//delete audioEngine;
	delete player;
	delete debugHUD;

	for (auto i : projectileList)
		delete i;
	projectileList.clear();
}

void NCL::CSC8503::PS5_Game::StartLevel()
{
	InitialisePlayerAssets();
	SpawnPlayer();
	InitializeProjectilePool();
	SpawnAI();
	InitTeleporters();
	SpawnDataDrivenLevel(GameLevelNumber::LEVEL_1);
	physics->createStaticTree();
	appState->SetIsGameOver(false);
	appState->SetIsGamePaused(false);
	timeElapsed = 0.0f;
}

void NCL::CSC8503::PS5_Game::EndLevel()
{
	appState->SetIsGameOver(true);
	world->ClearAndErase();
	physics->Clear();
	//delete player;
	player = nullptr;
	projectileList.clear();
	if (AIStateObject)
		AIStateObject = NULL;
	InitCamera();
}

void NCL::CSC8503::PS5_Game::UpdateGame(float dt)
{
	std::optional<time_point<high_resolution_clock>> frameStartTime;
	if (isDebuHUDActive)
		frameStartTime = high_resolution_clock::now();

	TutorialGame::UpdateGame(dt);

	Menu->Update(dt);

	if (!appState->GetIsGameOver())
	{
		/*if (timeElapsed > GAME_TIME_LIMIT) {
			ui->DrawStringText("Game Over", Vector2(5, 5), UIBase::RED);
			ui->DrawStringText("Score: " + std::to_string(player->GetScore()), Vector2(5, 10), UIBase::RED);
			ui->RenderUI(dt);
			appState->SetIsGameOver(true);
			appState->SetIsGamePaused(true);
			return;
		}*/

		/*ui->DrawStringText("Score: " + std::to_string(player->GetScore()), Vector2(5, 5), UIBase::RED);
		ui->DrawStringText("Bullets: " + std::to_string(player->GetNumBullets()), Vector2(5, 10), UIBase::RED);
		ui->DrawStringText("Time Left: " + std::to_string((int)(GAME_TIME_LIMIT - timeElapsed)), Vector2(5, 15), UIBase::RED);*/

		if (!appState->GetIsGamePaused())
		{
			timeElapsed += dt;

			physics->Update(dt);
			if (AIStateObject) {
				AIStateObject->DetectProjectiles(projectileList);
				AIStateObject->Update(dt);
			}

			timeSinceFire += dt;

			MovePlayer(dt);
			if (controller->GetNamedButtonAnalogue("R2") > 0.2f)
				Fire();

			player->ReplenishProjectiles(dt);
			gravitywell->PullProjectilesWithinField(projectileList);

			for (auto i : projectileList)
			{
				if (!i->IsActive()) continue;

				i->ReduceTimeLeft(dt);

				if (i->GetTimeLeft() <= 0)
					i->deactivate();
			}
		}

	}


	ui->RenderUI(dt);

	std::optional<time_point<high_resolution_clock>> frameEndTime;
	if (isDebuHUDActive)
		frameEndTime = high_resolution_clock::now();

	if (controller->GetNamedButton("Triangle"))
	{
		isDebuHUDActive = true;

		if (!frameStartTime.has_value() || !frameEndTime.has_value()) return;

		auto duration = duration_cast<microseconds>(frameEndTime.value() - frameStartTime.value());
		debugHUD->DrawDebugHUD({
			dt,
			duration.count(),
			physics->GetNumberOfCollisions(),
			world->GetNumberOfObjects()
		});
	}
}

void NCL::CSC8503::PS5_Game::InitializeProjectilePool()
{
	for (int i = 0; i < PROJECTILE_POOL_SIZE; i++) {
		Projectile* newBullet = new Projectile(player, this);

		float radius = 1.0f;
		Vector3 sphereSize = Vector3(radius, radius, radius);
		SphereVolume* volume = new SphereVolume(radius);
		newBullet->SetBoundingVolume((CollisionVolume*)volume);
		newBullet->GetTransform().SetScale(sphereSize).SetPosition(Vector3(0, 0, 0));
		newBullet->SetRenderObject(new RenderObject(&newBullet->GetTransform(), sphereMesh, basicTex, basicShader));
		newBullet->SetPhysicsObject(new PhysicsObject(&newBullet->GetTransform(), newBullet->GetBoundingVolume()));
		newBullet->GetPhysicsObject()->SetInverseMass(Projectile::inverseMass);
		newBullet->GetPhysicsObject()->InitSphereInertia();
		newBullet->GetRenderObject()->SetColour(Debug::RED);

		int bulletID = Projectile::CurrentAvailableProjectileID++;

		world->AddGameObject(newBullet);

		newBullet->GetPhysicsObject()->SetElasticity(1.0f);
		newBullet->GetPhysicsObject()->SetFriction(1.0f);
		newBullet->GetPhysicsObject()->SetFriction(1.0f);

		projectileList.push_back(newBullet);
		newBullet->deactivate();
	}
}

void NCL::CSC8503::PS5_Game::SpawnAI()
{
	AddAiStateObjectToWorld(Vector3(90, 5.6, 90));
}

AiStatemachineObject* NCL::CSC8503::PS5_Game::AddAiStateObjectToWorld(const Vector3& position) {
	NavigationGrid* navGrid = new NavigationGrid(world);
	AIStateObject = new AiStatemachineObject(world, navGrid);

	float radius = 4.0f;
	SphereVolume* volume = new SphereVolume(radius);
	AIStateObject->SetBoundingVolume((CollisionVolume*)volume);
	AIStateObject->GetTransform()
		.SetScale(Vector3(radius, radius, radius))
		.SetPosition(Vector3(position.x, 5.6, position.z));

	AIStateObject->SetRenderObject(new RenderObject(&AIStateObject->GetTransform(), sphereMesh, nullptr, basicShader));
	AIStateObject->SetPhysicsObject(new PhysicsObject(&AIStateObject->GetTransform(), AIStateObject->GetBoundingVolume()));

	AIStateObject->GetPhysicsObject()->SetInverseMass(1 / 10000000.0f);
	AIStateObject->GetPhysicsObject()->SetElasticity(0.000002);
	AIStateObject->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(AIStateObject);

	return AIStateObject;
}

void NCL::CSC8503::PS5_Game::SpawnProjectile(NetworkPlayer* player, Vector3 firePos, Vector3 fireDir)
{
	Projectile* newBullet = nullptr;
	for (auto i : projectileList) {
		if (i->IsActive()) continue;
		newBullet = i;
	}

	if (!newBullet) return;
	newBullet->activate();
	newBullet->ResetTime();

	newBullet->GetTransform().SetPosition(firePos);
	newBullet->GetPhysicsObject()->ClearForces();
	newBullet->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
	newBullet->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
	Vector3 force = fireDir * Projectile::FireForce;
	newBullet->GetPhysicsObject()->ApplyLinearImpulse(force);
}

void NCL::CSC8503::PS5_Game::SpawnPlayer()
{
	float meshSize = 5.0f;
	float inverseMass = 1.0f / 600000.0f;

	player = new NetworkPlayer(this, 0);
	SphereVolume* volume = new SphereVolume(1.6f);

	player->SetBoundingVolume((CollisionVolume*)volume);

	player->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(Vector3(0, 10, -75));

	player->SetRenderObject(new RenderObject(&player->GetTransform(), playerMesh, basicTex, playerShader));
	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));

	player->GetPhysicsObject()->SetInverseMass(inverseMass);
	player->GetPhysicsObject()->InitCubeInertia();

	player->GetRenderObject()->SetAnimation(*playerAnimation);

	world->AddGameObject(player);
}

void NCL::CSC8503::PS5_Game::InitialisePlayerAssets()
{
	playerMesh		= renderer->LoadMesh("Role_T.msh");
	playerAnimation = new MeshAnimation("Role_T.anm");
	playerShader	= renderer->LoadShader("scene.vert", "scene.frag");
}

void NCL::CSC8503::PS5_Game::MovePlayer(float dt) {
	float horizontalInput	= controller->GetNamedAxis("XLook");
	float verticalInput		= controller->GetNamedAxis("YLook");
	player->MovePlayerBasedOnController(dt, horizontalInput, verticalInput);

	float rotationX = controller->GetNamedAxis("RightX");
	float rotationY = -controller->GetNamedAxis("RightY");

	if (rotationX == 0 && rotationY == 0) return;

	player->RotatePlayerBasedOnController(dt, rotationX, rotationY);
}

void NCL::CSC8503::PS5_Game::Fire()
{
	if (timeSinceFire < 1.0f / FIRE_RATE) return;

	player->Fire();
	timeSinceFire = 0;
}

int NCL::CSC8503::PS5_Game::GetPlayerScore() const
{
	if (!player) return 0;

	return player->GetScore();
}

int NCL::CSC8503::PS5_Game::GEtPlayerBulletsNum() const
{
	if (!player) return 0;

	return player->GetNumBullets();
}
