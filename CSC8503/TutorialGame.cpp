#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "PhysicsSystem.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "Hole.h"
#include "BouncePad.h"
#include "GravityWell.h"
#include "Teleporter.h"
#include "NavigationGrid.h"
#include <Maths.h>
#include <cstdlib> 

#include <fstream>
#include <sstream>
#include <Helper.h>

#ifndef _WIN32
#include "PS5Window.h"
#endif

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() {
	world		= new GameWorld();
	world->ClearAndErase();
	physics = new PhysicsSystem(*world);
	physics->UseGravity(false);

#ifdef _WIN32
	controller = new KeyboardMouseController(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse());

	controller->MapAxis(0, "Sidestep");
	controller->MapAxis(1, "UpDown");
	controller->MapAxis(2, "Forward");

	controller->MapAxis(3, "XLook");
	controller->MapAxis(4, "YLook");

	world->GetMainCamera().SetController(*controller);

#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	ui = UIWindows::GetInstance();
#else
	renderer = new GameTechAGCRenderer(*world);

	controller = (dynamic_cast<PS5::PS5Window*>(Window::GetWindow()))->GetController();

	controller->MapAxis(0, "LeftX");
	controller->MapAxis(1, "LeftY");
	controller->MapAxis(2, "RightX");
	controller->MapAxis(3, "RightY");
	controller->MapAxis(4, "DX");
	controller->MapAxis(5, "DY");

	controller->MapButton(0, "Triangle");
	controller->MapButton(1, "Circle");
	controller->MapButton(2, "Cross");
	controller->MapButton(3, "Square");
	controller->MapButton(4, "Square");
	controller->MapButton(5, "");
	controller->MapButton(7, "R1");

	//These are the axis/button aliases the inbuilt camera class reads from:
	controller->MapAxis(0, "XLook");
	controller->MapAxis(1, "YLook");

	controller->MapAxis(2, "Sidestep");
	controller->MapAxis(3, "Forward");

	controller->MapButtonAnalogue(5, "R2");

	controller->MapButton(0, "Up");
	controller->MapButton(2, "Down");

	controller->MapButton(10, "Option");

	ui = UIPlaystation::GetInstance();
#endif
	
	levelFileLoader = new WindowsLevelLoader();
	currentlevel = level::level1;

	InitialiseAssets();

	appState = ApplicationState::GetInstance();

	BindEvents();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete wallMesh;
	delete bouncePlatformMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;
	delete basicTex;
	delete basicShader;
	delete pbrShader;
	delete blackholeTex;

	delete physics;
	delete renderer;
	delete world;

	delete levelFileLoader;

	SAFE_DELETE_PBR_TEXTURE(groundTextureList)
	SAFE_DELETE_PBR_TEXTURE(wallTextureList)
	SAFE_DELETE_PBR_TEXTURE(sandTextureList)
}

void NCL::CSC8503::TutorialGame::BindEvents()
{
	EventEmitter::RegisterForEvent(ACTIVATE_ICE_POWER_UP, this);
	EventEmitter::RegisterForEvent(ACTIVATE_WIND_POWER_UP, this);
	EventEmitter::RegisterForEvent(ACTIVATE_SAND_POWER_UP, this);
}

void NCL::CSC8503::TutorialGame::ReceiveEvent(EventType T)
{
	switch (T)
	{
	case ACTIVATE_ICE_POWER_UP:
		activePowerUpCount = Helper::Clamp(--activePowerUpCount, static_cast<unsigned int>(0), static_cast<unsigned int>(MAX_POWER_UP_COUNT));
		break;
	case ACTIVATE_SAND_POWER_UP:
		activePowerUpCount = Helper::Clamp(--activePowerUpCount, static_cast<unsigned int>(0), static_cast<unsigned int>(MAX_POWER_UP_COUNT));
		break;
	case ACTIVATE_WIND_POWER_UP:
		activePowerUpCount = Helper::Clamp(--activePowerUpCount, static_cast<unsigned int>(0), static_cast<unsigned int>(MAX_POWER_UP_COUNT));
		break;
	default:
		break;
	}
}

void TutorialGame::UpdateGame(float dt) {
	world->UpdateWorld(dt);
	renderer->Render();
	renderer->Update(dt);
	
	/*UpdatePowerUpSpawnTimer(dt);*/
}

//void NCL::CSC8503::TutorialGame::UpdatePowerUpSpawnTimer(float dt)
//{
//	if (appState->GetIsGameOver() || appState->GetIsGamePaused()) return;
//
//	powerUpSpawnTimer += dt;
//	if (powerUpSpawnTimer >= POWER_UP_SPAWN_TIME
//		&& activePowerUpCount <= MAX_POWER_UP_COUNT
//		)
//	{
//		powerUpSpawnTimer = 0.0f;
//		InitPowerup();
//	}
//}

void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	wallMesh = renderer->LoadMesh("cube.msh");
	bouncePlatformMesh = renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("sphere.msh");
	gooseMesh	= renderer->LoadMesh("goose.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	
	blackholeTex = renderer->LoadTexture("blackhole.jpg");
	basicTex	= renderer->LoadTexture("checkerboard.png");
	portalTex	= renderer->LoadTexture("PortalTex.jpg");
	sandTex		= renderer->LoadTexture("sand.jpg");
	targetTex = renderer->LoadTexture("blackhole.png");

	groundTextureList[(uint8_t)TextureType::ALBEDO] = renderer->LoadTexture("GrassWithRock01/albedo.png");
	groundTextureList[(uint8_t)TextureType::NORMAL] = renderer->LoadTexture("GrassWithRock01/normal_gl.png");
	groundTextureList[(uint8_t)TextureType::METAL] = renderer->LoadTexture("GrassWithRock01/metallic.png");
	groundTextureList[(uint8_t)TextureType::ROUGHNESS] = renderer->LoadTexture("GrassWithRock01/roughness.png");
	groundTextureList[(uint8_t)TextureType::AO] = renderer->LoadTexture("GrassWithRock01/ao.png");

	wallTextureList[(uint8_t)TextureType::ALBEDO] = renderer->LoadTexture("Metal_03/albedo.png");
	wallTextureList[(uint8_t)TextureType::NORMAL] = renderer->LoadTexture("Metal_03/normal_gl.png");
	wallTextureList[(uint8_t)TextureType::METAL] = renderer->LoadTexture("Metal_03/metallic.png");
	wallTextureList[(uint8_t)TextureType::ROUGHNESS] = renderer->LoadTexture("Metal_03/roughness.png");
	wallTextureList[(uint8_t)TextureType::AO] = renderer->LoadTexture("Metal_03/ao.png");

	sandTextureList[(uint8_t)TextureType::ALBEDO] = renderer->LoadTexture("GroundTile11/albedo.png");
	sandTextureList[(uint8_t)TextureType::NORMAL] = renderer->LoadTexture("GroundTile11/normal_gl.png");
	sandTextureList[(uint8_t)TextureType::METAL] = renderer->LoadTexture("GroundTile11/metallic.png");
	sandTextureList[(uint8_t)TextureType::ROUGHNESS] = renderer->LoadTexture("GroundTile11/roughness.png");
	sandTextureList[(uint8_t)TextureType::AO] = renderer->LoadTexture("GroundTile11/ao.png");

#ifdef defined(USE_SHADOW)
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
#else
	basicShader = renderer->LoadShader("scene.vert", "sceneNoShadow.frag");
#endif // USE_SHADOW

	pbrShader = renderer->LoadShader("pbr.vert", "pbr.frag");
	portalShader = renderer->LoadShader("scene.vert", "portal.frag");
	instancePbrShader = renderer->LoadShader("pbrInstanced.vert", "pbr.frag");
	blackholeShader = renderer->LoadShader("blackhole.vert", "blackhole.frag");
	targetholeShader = renderer->LoadShader("targethole.vert", "targethole.frag");
	particleShader = renderer->LoadShader("particleDefault.vert", "particleMoving.frag");

	InitCamera();
	InitWorld();
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-90.0f);
	world->GetMainCamera().SetYaw(0.0f);
	world->GetMainCamera().SetPosition(Vector3(0, 245, 0));
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();
	InitCamera();
	physics->UseGravity(false);
	physics->SetBroadphase(true);
	timer = 0;
}

PowerUp* TutorialGame::InitPowerup()
{
	PowerUp* tempPowerup = new PowerUp();

	SphereVolume* tempSphereVolume = new SphereVolume(1.5, true , false);
	tempPowerup->SetBoundingVolume((CollisionVolume*)tempSphereVolume);

	tempPowerup->GetTransform()
		.SetPosition(Helper::GetRandomDataFromVector(powerUpSpawnPointList))
		.SetScale(Vector3(tempSphereVolume->GetRadius(), tempSphereVolume->GetRadius(), tempSphereVolume->GetRadius()) * 2);

	tempPowerup->SetPhysicsObject(new PhysicsObject(&tempPowerup->GetTransform(), tempPowerup->GetBoundingVolume()));

	tempPowerup->GetPhysicsObject()->SetInverseMass(0);
	tempPowerup->GetPhysicsObject()->InitSphereInertia();
	tempPowerup->GetPhysicsObject()->SetElasticity(0.5);

	(this->*powerupInitFunctionList[Helper::GetRandomEnumValue(powerUpType::MAX_POWERUP)])(tempPowerup, pbrShader);
	world->AddGameObject(tempPowerup);

	return tempPowerup;
}

void NCL::CSC8503::TutorialGame::InitNonePowerup(PowerUp* inPowerup, Shader* inShader)
{
}

void NCL::CSC8503::TutorialGame::InitIcePowerup(PowerUp* inPowerup, Shader* inShader)
{
	inPowerup->SetRenderObject(new RenderObject(&inPowerup->GetTransform(), sphereMesh, basicTex, inShader));
	inPowerup->setPowerup(powerUpType::ice);
	activePowerUpCount = Helper::Clamp(++activePowerUpCount, static_cast<unsigned int>(0), static_cast<unsigned int>(MAX_POWER_UP_COUNT));
	for (size_t i = 0; i < (uint8_t)TextureType::MAX_TYPE; i++)
	{
		inPowerup->GetRenderObject()->SetTexture((TextureType)i, groundTextureList[i]);
	}
}

void NCL::CSC8503::TutorialGame::InitSandPowerup(PowerUp* inPowerup, Shader* inShader)
{
	inPowerup->SetRenderObject(new RenderObject(&inPowerup->GetTransform(), sphereMesh, basicTex, inShader));
	inPowerup->setPowerup(powerUpType::sand);
	activePowerUpCount = Helper::Clamp(++activePowerUpCount, static_cast<unsigned int>(0), static_cast<unsigned int>(MAX_POWER_UP_COUNT));
	for (size_t i = 0; i < (uint8_t)TextureType::MAX_TYPE; i++)
	{
		inPowerup->GetRenderObject()->SetTexture((TextureType)i, groundTextureList[i]);
	}

}

void NCL::CSC8503::TutorialGame::InitWindPowerup(PowerUp* inPowerup, Shader* inShader)
{
	inPowerup->SetRenderObject(new RenderObject(&inPowerup->GetTransform(), sphereMesh, basicTex, inShader));
	inPowerup->setPowerup(powerUpType::wind);
	activePowerUpCount = Helper::Clamp(++activePowerUpCount, static_cast<unsigned int>(0), static_cast<unsigned int>(MAX_POWER_UP_COUNT));
	for (size_t i = 0; i < (uint8_t)TextureType::MAX_TYPE; i++)
	{
		inPowerup->GetRenderObject()->SetTexture((TextureType)i, groundTextureList[i]);
	}
}


GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, const Vector3& size) {

	GameObject *floor = new GameObject();

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize , false ,true);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, pbrShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);


	return floor;
}

void TutorialGame::SpawnDataDrivenLevel(GameLevelNumber inGameLevelNumber)
{
	std::ifstream input{ levelFileLoader->GetLevelFilePath(inGameLevelNumber)};

	if (!input.is_open()) {
		std::cerr <<"[TutorialGame::SpawnDataDrivenLevel] Couldn't read file: " << levelFileLoader->GetLevelFilePath(inGameLevelNumber) << "\n";
		return;
	}

	float tempLevelNodeData[12];
	int tempIndex = 0;
	for (std::string line; std::getline(input, line);) 
	{
		tempIndex = 0;
		std::istringstream ss(std::move(line));
		for (std::string value; std::getline(ss, value, ',');) 
		{
			tempLevelNodeData[tempIndex++] = std::stof(value);
		}

		Vector3 tempPosition = Vector3(tempLevelNodeData[1], tempLevelNodeData[2], tempLevelNodeData[3]);
		Vector3 tempRotation = Vector3(tempLevelNodeData[4], tempLevelNodeData[5], tempLevelNodeData[6]);
		Vector3 tempScale = Vector3(tempLevelNodeData[7], tempLevelNodeData[8], tempLevelNodeData[9]) / 2.0f;
		Vector2 tempTiling = Vector2(tempLevelNodeData[10], tempLevelNodeData[11]);

		(this->*levelObjectSpawnFunctionList[(int)tempLevelNodeData[0]])(tempPosition, tempRotation, tempScale, tempTiling);
	}
}

void NCL::CSC8503::TutorialGame::SpawnWall(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling)
{
	Matrix4 tempTransform = Matrix4();
	tempTransform.SetPositionVector(inPosition);
	tempTransform.SetDiagonal(inScale * 2);

	wallMesh->AddInstanceModelMatrices(tempTransform);

	GameObject* tempWall = AddAABBCubeToWorld(
		inPosition,
		inScale,
		0, 0.5f);
	tempWall->GetRenderObject()->SetShader(instancePbrShader);
	tempWall->GetRenderObject()->SetTiling(inTiling);
	tempWall->setName("wall");
	tempWall->GetRenderObject()->SetMesh(wallMesh);

	for (size_t i = 0; i < (uint8_t)TextureType::MAX_TYPE; i++)
	{
		tempWall->GetRenderObject()->SetTexture((TextureType)i, wallTextureList[i]);
	}
}

void NCL::CSC8503::TutorialGame::SpawnFloor(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling)
{
	GameObject* tempFloor = AddFloorToWorld(
		inPosition,
		inScale);
	tempFloor->GetRenderObject()->SetTiling(inTiling);
	tempFloor->settag("floor1");

	for (size_t i = 0; i < (uint8_t)TextureType::MAX_TYPE; i++)
	{
		tempFloor->GetRenderObject()->SetTexture((TextureType)i, sandTextureList[i]);
	}
}

void NCL::CSC8503::TutorialGame::SpawnBouncingPad(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling)
{
	BouncePad* tempBouncePad = new BouncePad(bouncePlatformMesh, basicTex, instancePbrShader, inScale * 2);
	for (size_t i = 0; i < (uint8_t)TextureType::MAX_TYPE; i++)
	{
		tempBouncePad->GetRenderObject()->SetTexture((TextureType)i, groundTextureList[i]);
	}
	tempBouncePad->GetRenderObject()->SetTiling(inTiling);
	world->AddGameObject(tempBouncePad);
	tempBouncePad->GetTransform().SetPosition(inPosition).SetOrientation(Quaternion::EulerAnglesToQuaternion(inRotation.x, inRotation.y, inRotation.z));
	bouncePlatformMesh->AddInstanceModelMatrices(tempBouncePad->GetTransform().GetMatrix());

}

void NCL::CSC8503::TutorialGame::SpawnTarget(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling)
{

	Hole* hole = new Hole();

	float radius = 2.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	hole->SetBoundingVolume((CollisionVolume*)volume);
	hole->GetTransform().SetScale(inScale).SetPosition(inPosition).SetOrientation(Quaternion::EulerAnglesToQuaternion(inRotation.x, inRotation.y, inRotation.z));
	hole->SetPhysicsObject(new PhysicsObject(&hole->GetTransform(), hole->GetBoundingVolume()));
	hole->GetPhysicsObject()->SetInverseMass(0);
	hole->GetPhysicsObject()->InitSphereInertia();

	GameObject* targetDisplay = new GameObject();
	targetDisplay->GetTransform()
		.SetPosition(Vector3(inPosition.x, -3.0, inPosition.z))
		.SetScale(Vector3(inScale.x, inScale.y, inScale.z))
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(inRotation.x, inRotation.y, inRotation.z));
	targetDisplay->SetRenderObject(new RenderObject(&targetDisplay->GetTransform(), cubeMesh, targetTex, targetholeShader));

	world->AddGameObject(hole);
	world->AddGameObject(targetDisplay);
}

void NCL::CSC8503::TutorialGame::SpawnBlackHole(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling)
{
	//GravityWell
	GravityWell* newgravityWell = new GravityWell();

	float radius = 1.5f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	newgravityWell->SetBoundingVolume((CollisionVolume*)volume);
	newgravityWell->GetTransform().SetScale(inScale).SetPosition(inPosition).SetOrientation(Quaternion::EulerAnglesToQuaternion(inRotation.x, inRotation.y, inRotation.z));
	
	newgravityWell->SetPhysicsObject(new PhysicsObject(&newgravityWell->GetTransform(), newgravityWell->GetBoundingVolume()));
	newgravityWell->GetPhysicsObject()->SetInverseMass(0);
	newgravityWell->GetPhysicsObject()->InitSphereInertia();
	
	GameObject* blackholeDisplay = new GameObject();
	blackholeDisplay->GetTransform()
		.SetPosition(Vector3(inPosition.x, -2.0, inPosition.z))
		.SetScale(Vector3(inScale.x, inScale.y, inScale.z))
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(inRotation.x, inRotation.y, inRotation.z));
	blackholeDisplay->SetRenderObject(new RenderObject(&blackholeDisplay->GetTransform(), sphereMesh, blackholeTex, blackholeShader));

	gravitywell.push_back(newgravityWell);
	world->AddGameObject(newgravityWell);
	world->AddGameObject(blackholeDisplay);
}

void NCL::CSC8503::TutorialGame::AddPowerUpSpawnPoint(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling)
{
	powerUpSpawnPointList.emplace_back(inPosition);
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float radius, float halfHeight, float inverseMass, float elasticity) {

	GameObject* capsule = new GameObject("capsule");

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius, false, true);
	capsule->SetBoundingVolume((CollisionVolume*)volume);
	Vector3 capsuleSize = Vector3(2 * radius, 2 * halfHeight, 2 * radius);

	capsule->GetTransform()
		.SetPosition(position)
		.SetScale(capsuleSize);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(0);
	capsule->GetPhysicsObject()->InitCubeInertia();
	capsule->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(capsule);


	return capsule;
}

GameObject* TutorialGame::AddObbCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity) {

	 cube = new GameObject("cube");

	OBBVolume* volume = new OBBVolume(dimensions,false, true);
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

	AABBVolume* volume = new AABBVolume(dimensions, false ,true);
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

GameObject* NCL::CSC8503::TutorialGame::AddTeleporterToWorld(const Vector3& position1,const Vector3& position2,const Vector3& rotation1 , const Vector3& rotation2, Vector3 dimensions, float inverseMass, float elasticity)
{

	Teleporter* teleporter1 = new Teleporter();
	OBBVolume* volume1 = new OBBVolume(dimensions, true, true);
	teleporter1->SetBoundingVolume((CollisionVolume*)volume1);
	teleporter1->GetTransform()
		.SetPosition(position1)
		.SetScale(dimensions * 2)
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(rotation1.x,rotation1.y,rotation1.z));
	teleporter1->setOutNormal(teleporter1->GetTransform().GetOrientation().ToEuler());
	//teleporter1->SetRenderObject(new RenderObject(&teleporter1->GetTransform(), cubeMesh, basicTex, basicShader));
	//teleporter1->GetRenderObject()->SetColour(Vector4(0.2f, 0.2f, 0.2f, 0.5f));
	teleporter1->SetPhysicsObject(new PhysicsObject(&teleporter1->GetTransform(), teleporter1->GetBoundingVolume()));
	teleporter1->GetPhysicsObject()->SetInverseMass(inverseMass);
	teleporter1->GetPhysicsObject()->InitCubeInertia();
	teleporter1->GetPhysicsObject()->SetElasticity(elasticity);

	float maxLength = std::max(dimensions.x, dimensions.z);
	GameObject* teleporter1Display = new GameObject();
	teleporter1Display->GetTransform()
		.SetPosition(position1)
		.SetScale(Vector3(maxLength * 2, 0.01, maxLength * 2))
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(rotation1.x, rotation1.y, rotation1.z));
	teleporter1Display->SetRenderObject(new RenderObject(&teleporter1Display->GetTransform(), cubeMesh, portalTex, portalShader));


	Teleporter* teleporter2 = new Teleporter(teleporter1);
	OBBVolume* volume2 = new OBBVolume(dimensions, true, true);
	teleporter2->SetBoundingVolume((CollisionVolume*)volume2);
	teleporter2->GetTransform()
		.SetPosition(position2)
		.SetScale(dimensions * 2)
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(rotation2.x, rotation2.y, rotation2.z));
	teleporter2->setOutNormal(teleporter2->GetTransform().GetOrientation().ToEuler());
	teleporter2->SetPhysicsObject(new PhysicsObject(&teleporter2->GetTransform(), teleporter2->GetBoundingVolume()));
	teleporter2->GetPhysicsObject()->SetInverseMass(inverseMass);
	teleporter2->GetPhysicsObject()->InitCubeInertia();
	teleporter2->GetPhysicsObject()->SetElasticity(elasticity);

	GameObject* teleporter2Display = new GameObject();
	teleporter2Display->GetTransform()
		.SetPosition(position2)
		.SetScale(Vector3(maxLength * 2, 0.01, maxLength * 2))
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(rotation2.x, rotation2.y, rotation2.z));
	teleporter2Display->SetRenderObject(new RenderObject(&teleporter2Display->GetTransform(), cubeMesh, portalTex, portalShader));

	//teleporter1->setConnectedTeleporter(teleporter2);
	//teleporter2->setConnectedTeleporter(teleporter1);

	world->AddGameObject(teleporter1);
	world->AddGameObject(teleporter1Display);
	world->AddGameObject(teleporter2);
	world->AddGameObject(teleporter2Display);

	return teleporter1;
}

void NCL::CSC8503::TutorialGame::InitTeleporters()
{
	AddTeleporterToWorld((Vector3(60, 5.0f, -41)), (Vector3(-61, 5.0f, 40)), Vector3(0, -45, 0), Vector3(0, 90, 0) , Vector3(10, 10, 3.5));
}

void TutorialGame::TestAddStaticObjectsToWorld() {
	for (int x = 0; x < 25; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 25; z++) {
				Vector3 position = Vector3(-50 + (x * 4), 5 +(y*4), -50 + z * 4);
				AddAABBCubeToWorld(position, Vector3(0.99,0.99,0.99), 0.0f, 0);
			}
		}
	}
}




