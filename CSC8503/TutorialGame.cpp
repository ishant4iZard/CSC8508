#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "Hole.h"
#include "BouncePad.h"
#include "GravityWell.h"

#include <Maths.h>
#include <cstdlib> 

using namespace NCL;
using namespace CSC8503;



TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	useGravity		= true;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");
	currentlevel = level::level1;

	gameover = false;

	InitialiseAssets();

#ifdef  _WIN32
	ui = UIWindows::GetInstance();
#endif //  _WIN32
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("sphere.msh");
	gooseMesh	= renderer->LoadMesh("goose.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	sandTex		= renderer->LoadTexture("sand.jpg");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
	
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	
	if (gameover) {
		world->ClearAndErase();
		physics->Clear();
		InitCamera();
		//Debug::Print("GameOver", Vector2(40, 10), Debug::RED);
		//Debug::Print("Final Score:" + std::to_string((int)(score)), Vector2(40, 30));
		//Debug::Print("Press 'ESC' to exit", Vector2(40, 50));

		ui->DrawStringText("GameOver", Vector2(40, 10), UIBase::RED);
		ui->DrawStringText("Final Score:" + std::to_string((int)(score)), Vector2(40, 30));
		ui->DrawStringText("Press 'ESC' to exit", Vector2(40, 50), UIBase::WHITE);

	}
	else if (gameWon)
	{
		world->ClearAndErase();
		physics->Clear();
		InitCamera();
		/*Debug::Print("You Won!!!", Vector2(40, 10), Debug::GREEN);
		Debug::Print("You saved the world from goats!!!", Vector2(40, 20), Debug::GREEN);
		Debug::Print("Final Score:" + std::to_string((int)(score + (300 - finaltimer) / 10)), Vector2(40, 30));
		Debug::Print("Finish Time:" + std::to_string((int)(finaltimer)), Vector2(40, 40));
		Debug::Print("Press 'ESC' to exit", Vector2(40, 50));*/

		ui->DrawStringText("You Won!!!", Vector2(40, 10), UIBase::GREEN);
		ui->DrawStringText("You saved the world from goats!!!", Vector2(40, 20), UIBase::GREEN);
		ui->DrawStringText("Final Score:" + std::to_string((int)(score + (300 - finaltimer) / 10)), Vector2(40, 30), UIBase::YELLOW);
		ui->DrawStringText("Finish Time:" + std::to_string((int)(finaltimer)), Vector2(40, 40), UIBase::YELLOW);
		ui->DrawStringText("Press 'ESC' to exit", Vector2(40, 50), UIBase::YELLOW);
	}

	else if(serverStarted) {
		timer += dt;

		world->GetMainCamera().UpdateCamera(dt);

		if (timer > TIME_LIMIT) {
			gameover = true;
		}


		std::string timeText = "Time left : " + std::to_string((int)(TIME_LIMIT - timer));
		ui->DrawStringText(timeText, Vector2(5, 10));
	}
	world->UpdateWorld(dt);
	renderer->Update(dt);
	renderer->Render();

}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-90.0f);
	world->GetMainCamera().SetYaw(0.0f);
	world->GetMainCamera().SetPosition(Vector3(0, 245, 0));
}

void TutorialGame::InitWorld() {
	gameover = false;
	world->ClearAndErase();
	physics->Clear();
	InitCamera();
	physics->UseGravity(false);
	physics->SetBroadphase(true);
	timer = 0;

	InitDefaultFloor();
	InitBouncePad();
	InitLevelWall();
	InitPlaceholderAIs();
	InitHole();
	InitGravityWell();
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/

GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const Vector3& size) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/


void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0), Vector3(100, 2, 100));
}

void TutorialGame::InitHole() {
	Hole* hole = new Hole();

	float radius = 1.5f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	hole->SetBoundingVolume((CollisionVolume*)volume);
	hole->GetTransform().SetScale(sphereSize).SetPosition(Vector3(0, 3, 0));
	hole->SetRenderObject(new RenderObject(&hole->GetTransform(), sphereMesh, basicTex, basicShader));
	hole->SetPhysicsObject(new PhysicsObject(&hole->GetTransform(), hole->GetBoundingVolume()));
	hole->GetPhysicsObject()->SetInverseMass(0);
	hole->GetPhysicsObject()->InitSphereInertia();

	hole->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
	
	world->AddGameObject(hole);
}

void NCL::CSC8503::TutorialGame::InitGravityWell()
{
	GravityWell* gravityWell = new GravityWell();

	float radius = 1.5f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	gravityWell->SetBoundingVolume((CollisionVolume*)volume);
	gravityWell->GetTransform().SetScale(sphereSize).SetPosition(Vector3(15, 3, 15));
	gravityWell->SetRenderObject(new RenderObject(&gravityWell->GetTransform(), sphereMesh, basicTex, basicShader));
	gravityWell->SetPhysicsObject(new PhysicsObject(&gravityWell->GetTransform(), gravityWell->GetBoundingVolume()));
	gravityWell->GetPhysicsObject()->SetInverseMass(0);
	gravityWell->GetPhysicsObject()->InitSphereInertia();

	gravityWell->GetRenderObject()->SetColour(Vector4(0, 0.4, 0.4, 1));

	gravitywell = gravityWell;
	world->AddGameObject(gravityWell);
}

GameObject* TutorialGame::AddObbCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity) {
	GameObject* cube = new GameObject("cube");

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(cube);

	return cube;
}
GameObject* TutorialGame::AddAABBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity) {
	GameObject* cube = new GameObject("cube");

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(cube);

	return cube;
}



void TutorialGame::InitBouncePad()
{
	for (size_t i = 0; i < 5; i++)
	{
		BouncePad* tempBouncePad = new BouncePad(cubeMesh, basicTex, basicShader);
		tempBouncePad->GetRenderObject()->SetColour(Debug::CYAN);
		bouncePadList[i] = tempBouncePad;
		world->AddGameObject(bouncePadList[i]);
	}

	bouncePadList[0]->GetTransform().SetPosition(Vector3(48, 0, 0));
	bouncePadList[1]->GetTransform().SetPosition(Vector3(-48, 0, 0));
	bouncePadList[2]->GetTransform().SetPosition(Vector3(48, 0, 48));
	bouncePadList[3]->GetTransform().SetPosition(Vector3(48, 0, -48));
	bouncePadList[4]->GetTransform().SetPosition(Vector3(-30, 0, 48));

}

void TutorialGame::InitLevelWall()
{
	AddAABBCubeToWorld(Vector3(96,0,0), Vector3(10, 20, 100), 0, 0.5f);
	AddAABBCubeToWorld(Vector3(-96, 0, 0), Vector3(10, 20, 100), 0, 0.5f);
	AddAABBCubeToWorld(Vector3(0,0, 96), Vector3(100, 20, 10), 0, 0.5f);
	AddAABBCubeToWorld(Vector3(0, 0, -96), Vector3(100, 20, 10), 0, 0.5f);
}

void TutorialGame::InitPlaceholderAIs() {
	const static int NUMBER_OF_AIS = 5;
	const static Vector3 AI_SCALE = Vector3(3, 3, 3);
	const static int GAME_ARENA_LENGTH = 65;
	const static int ENEMY_AI_WEIGHT = 0;
	const static int ENEMY_AI_ELASTICITY = 0.1;

	for (size_t i = 0; i < NUMBER_OF_AIS; i++)
	{
		Vector3 randomPosition = Vector3(
			(std::rand() % (2 * GAME_ARENA_LENGTH)) - GAME_ARENA_LENGTH, // Random value between -length to length
			0, 
			(std::rand() % (2 * GAME_ARENA_LENGTH)) - GAME_ARENA_LENGTH);
		placeHolderAIs.push_back(AddObbCubeToWorld(randomPosition, AI_SCALE, ENEMY_AI_WEIGHT, ENEMY_AI_ELASTICITY));
		placeHolderAIs[i]->GetRenderObject()->SetColour(Debug::RED);
	}
}
