#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
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


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	aitreetest = new AiTreeObject("aitree");
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

#ifdef _WIN32
	levelFileLoader = new WindowsLevelLoader();
#endif // _WIN32

	//useGravity		= false;
	physics->UseGravity(false);

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");
	currentlevel = level::level1;


	InitialiseAssets();
	//GameObject* test = AddAABBCubeToWorld(Vector3(0, 1, 0), Vector3(100, 1, 100), 1 / 100, 0.1f);
	//test->settag("walls");

#ifdef _WIN32
	ui = UIWindows::GetInstance();
#else //_ORBIS
	ui = UIPlaystation::GetInstance();
#endif
	appState = ApplicationState::GetInstance();
	bm = new OGLTextureManager();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
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
	
	blackholeTex = renderer->LoadTexture("blackhole.png");
	basicTex	= renderer->LoadTexture("checkerboard.png");
	portalTex	= renderer->LoadTexture("PortalTex.jpg");
	sandTex		= renderer->LoadTexture("sand.jpg");

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

	sandTextureList[(uint8_t)TextureType::ALBEDO] = renderer->LoadTexture("Sand_02/albedo.png");
	sandTextureList[(uint8_t)TextureType::NORMAL] = renderer->LoadTexture("Sand_02/normal_gl.png");
	sandTextureList[(uint8_t)TextureType::METAL] = renderer->LoadTexture("Sand_02/metallic.png");
	sandTextureList[(uint8_t)TextureType::ROUGHNESS] = renderer->LoadTexture("Sand_02/roughness.png");
	sandTextureList[(uint8_t)TextureType::AO] = renderer->LoadTexture("Sand_02/ao.png");

#ifdef defined(USE_SHADOW)
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
#else
	basicShader = renderer->LoadShader("scene.vert", "sceneNoShadow.frag");
#endif // USE_SHADOW

	pbrShader = renderer->LoadShader("pbr.vert", "pbr.frag");
	portalShader = renderer->LoadShader("scene.vert", "portal.frag");
	instancePbrShader = renderer->LoadShader("pbrInstanced.vert", "pbr.frag");
	blackholeShader = renderer->LoadShader("blackhole.vert", "blackhole.frag");

	InitCamera();
	InitWorld();

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

	delete[] groundTextureList;
	delete[] wallTextureList;
	delete[] sandTextureList;
	delete aitreetest;
}

void TutorialGame::UpdateGame(float dt) {
	
	//ObjectRay(testStateObject, testGameObjects);

	if (appState->GetIsGameOver()) {
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
	else if (appState->GetHasWonGame())
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

	else if(appState->GetIsServer()) {
		timer += dt;

		world->GetMainCamera().UpdateCamera(dt);
		if (timer > TIME_LIMIT) {
			appState->SetIsGameOver(true);
		}

		//Debug::UpdateRenderables(dt);

		std::string timeText = "Time left : " + std::to_string((int)(TIME_LIMIT - timer));
		ui->DrawStringText(timeText, Vector2(5, 10));
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::C)) {
		//claptrap->OnAwake(2);
		aitreetest->ResetBehaviourTree();
		aitreetest->OnBehaviour();
	}

	//if (aitreetest) {
	//	aitreetest-> Update(dt);
	//}

	//testStateObject->getPositionfromobject()
	//if (testStateObject) {
	//	testStateObject->Update(dt);
	//}
	
	static int frameCounter = 0;
	//aitreetest->GetTransform().RandomPosition(aitreetest->GetTransform().GetPosition(), true).SetScale(Vector3(10, 10, 10) * 2);
	frameCounter++;


	world->UpdateWorld(dt);
	renderer->Render();
	renderer->Update(dt);
	//std::cout << "capsule" << capsule->GetTransform().GetPosition().y<<"\n";


	if (frameCounter >= 100) {
		aitreetest->GetTransform().RandomPosition(aitreetest->GetTransform().GetPosition(), true);
		Vector3 aichaseposition = aitreetest->GetTransform().GetPosition();
		frameAddresses.push_back(aichaseposition);
		//ProcessFrameAddresses();
		frameCounter = 0; 
	}

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
	AddCapsuleToWorld(Vector3(-75, 10, -75), 2.0f, 5.0f);
	/*AddCapsuleToWorld(Vector3(-40, 10, -75), 2.0f, 5.0f);
	AddCapsuleToWorld(Vector3(-35, 10, -75), 2.0f, 5.0f);
	AddCapsuleToWorld(Vector3(-15, 10, -75), 2.0f, 5.0f);
	AddCapsuleToWorld(Vector3(-75, 10, -75), 2.0f, 5.0f);
	AddCapsuleToWorld(Vector3(-70, 10, -70), 2.0f, 5.0f);
	AddCapsuleToWorld(Vector3(-65, 10, -65), 2.0f, 5.0f);*/
	capsule = AddCapsuleToWorld(Vector3(-80, 5.6, -80), 1.0f, 2.0f);

	SpawnDataDrivenLevel(GameLevelNumber::LEVEL_1);
	//capsule->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, 0, 90));

	InitTeleporters();
	InitBlackhole();
	//TestAddStaticObjectsToWorld();
	//InitAI();

	GameObject* test = AddCapsuleToWorld(Vector3(-25, 5.6, 25), 1.0f, 2.0f);
	//test->SetRenderObject();
	physics->createStaticTree();//this needs to be at the end of all initiations
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/

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
	BouncePad* tempBouncePad = new BouncePad(bouncePlatformMesh, basicTex, instancePbrShader);
	for (size_t i = 0; i < (uint8_t)TextureType::MAX_TYPE; i++)
	{
		tempBouncePad->GetRenderObject()->SetTexture((TextureType)i, groundTextureList[i]);
	}
	tempBouncePad->GetRenderObject()->SetTiling(inTiling);
	world->AddGameObject(tempBouncePad);

	tempBouncePad->GetTransform().SetPosition(inPosition);

	bouncePlatformMesh->AddInstanceModelMatrices(tempBouncePad->GetTransform().GetMatrix());
}

void NCL::CSC8503::TutorialGame::SpawnTarget(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling)
{
	Hole* hole = new Hole();

	float radius = 2.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	hole->SetBoundingVolume((CollisionVolume*)volume);
	hole->GetTransform().SetScale(sphereSize).SetPosition(inPosition);
	hole->SetRenderObject(new RenderObject(&hole->GetTransform(), sphereMesh, basicTex, basicShader));
	hole->SetPhysicsObject(new PhysicsObject(&hole->GetTransform(), hole->GetBoundingVolume()));
	hole->GetPhysicsObject()->SetInverseMass(0);
	hole->GetPhysicsObject()->InitSphereInertia();

	hole->GetRenderObject()->SetColour(Vector4(0, 0, 0, 1));
	hole->GetRenderObject()->SetTiling(inTiling);
	world->AddGameObject(hole);
}

void NCL::CSC8503::TutorialGame::SpawnBlackHole(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling)
{
	//GravityWell
	GravityWell* gravityWell = new GravityWell();

	float radius = 1.5f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	gravityWell->SetBoundingVolume((CollisionVolume*)volume);
	gravityWell->GetTransform().SetScale(inScale).SetPosition(inPosition).SetOrientation(Quaternion::EulerAnglesToQuaternion(inRotation.x, inRotation.y, inRotation.z));
	gravityWell->SetRenderObject(new RenderObject(&gravityWell->GetTransform(), sphereMesh, blackholeTex, blackholeShader));
	gravityWell->SetPhysicsObject(new PhysicsObject(&gravityWell->GetTransform(), gravityWell->GetBoundingVolume()));
	gravityWell->GetPhysicsObject()->SetInverseMass(0);
	gravityWell->GetPhysicsObject()->InitSphereInertia(); 
	
	gravitywell = gravityWell;
	world->AddGameObject(gravityWell);
}
/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/


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

void TutorialGame::InitDefaultFloor() {
	//AddFloorToWorld(Vector3(0, -2, 0), Vector3(100, 2, 100));
	//AddAiToWorld(Vector3(0, -2, 0), Vector3(10, 10, 10), 1.0, 1.0);
}

void TutorialGame::InitHole() {
	Hole* hole = new Hole();

	float radius = 1.5f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius ,false ,true);
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
	SphereVolume* volume = new SphereVolume(radius , false, true);
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

	world->AddGameObject(teleporter1);
	world->AddGameObject(teleporter1Display);
	world->AddGameObject(teleporter2);
	world->AddGameObject(teleporter2Display);

	return teleporter1;
}

//AiTreeObject* TutorialGame::AddAiToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity) {
//	aitreetest = new AiTreeObject("Aitreeobject");
//
//	OBBVolume* volume = new OBBVolume(dimensions);
//	aitreetest->SetBoundingVolume((CollisionVolume*)volume);
//
//	//aitreetest->GetTransform().RandomPosition(position, true).SetScale(dimensions * 2);
//		//.SetPosition(position).SetScale(dimensions * 2);
//
//	aitreetest->SetRenderObject(new RenderObject(&aitreetest->GetTransform(), cubeMesh, basicTex, basicShader));
//	aitreetest->SetPhysicsObject(new PhysicsObject(&aitreetest->GetTransform(), aitreetest->GetBoundingVolume()));
//
//	aitreetest->GetPhysicsObject()->SetInverseMass(inverseMass);
//	aitreetest->GetPhysicsObject()->InitCubeInertia();
//	aitreetest->GetPhysicsObject()->SetElasticity(elasticity);
//	aitreetest->BehaviorTree();
//
//	world->AddGameObject(aitreetest);
//	return aitreetest;
//
//}

void TutorialGame::InitBouncePad()
{
	/*
	for (size_t i = 0; i < 5; i++)
	{
		BouncePad * tempBouncePad = new BouncePad(cubeMesh, basicTex, basicShader);
		tempBouncePad->GetRenderObject()->SetColour(Debug::CYAN);
		bouncePadList[i] = tempBouncePad;
		world->AddGameObject(bouncePadList[i]);
	}

	bouncePadList[0]->GetTransform().SetPosition(Vector3(48, 0, 0));
	bouncePadList[1]->GetTransform().SetPosition(Vector3(-48, 0, 0));
	bouncePadList[2]->GetTransform().SetPosition(Vector3(48, 0, 48));
	bouncePadList[3]->GetTransform().SetPosition(Vector3(48, 0, -48));
	bouncePadList[4]->GetTransform().SetPosition(Vector3(-30, 0, 48));*/

}

void TutorialGame::InitLevelWall()
{
	AddAABBCubeToWorld(Vector3(96,0,0), Vector3(10, 20, 100), 0, 0.5f);
	AddAABBCubeToWorld(Vector3(-96, 0, 0), Vector3(10, 20, 100), 0, 0.5f);
	AddAABBCubeToWorld(Vector3(0,0, 96), Vector3(100, 20, 10), 0, 0.5f);
	AddAABBCubeToWorld(Vector3(0, 0, -96), Vector3(100, 20, 10), 0, 0.5f);
}

void NCL::CSC8503::TutorialGame::InitTeleporters()
{
	AddTeleporterToWorld((Vector3(48, 5.6f, 0)), (Vector3(-48, 5.6f, 45)), Vector3(0, -45, 0), Vector3(0, 90, 0) , Vector3(10, 10, 3.5));
}

void NCL::CSC8503::TutorialGame::InitBlackhole()
{
	//SpawnBlackHole(Vector3(-25, 5.6f, 25), (Vector3(-90, 180, 40)), Vector3(50.0f, 50.0f, 50.0f), Vector2(1, 1));
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
		
		placeHolderAIs.push_back(AddAABBCubeToWorld(randomPosition, AI_SCALE, ENEMY_AI_WEIGHT, ENEMY_AI_ELASTICITY));
		placeHolderAIs[i]->GetRenderObject()->SetColour(Debug::RED);
		placeHolderAIs[i]->settag("walls");
	}
}

void TutorialGame::ProcessFrameAddresses() {

	for (auto address : frameAddresses) {

		std::cout << "Address: " << address << std::endl;
	}
	//frameAddresses.clear();
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


//void TutorialGame::ObjectRay(GameObject* gameObject, GameObject* gameObject2) {
//
//	Vector3 objectPosition = gameObject->GetTransform().GetPosition() + Vector3(0, 0, 10);
//	Vector3 objectForward = gameObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
//	Ray ray(objectPosition, objectForward);
//
//	RayCollision closestCollision;
//	closestCollision.rayDistance = 100.0f;
//
//	if (world->Raycast(ray, closestCollision, true, gameObject)) {
//		if (closestCollision.node == gameObject2) {
//			Debug::DrawLine(objectPosition, objectForward * 100, Debug::BLACK);
//			gameObject2->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * 100, closestCollision.collidedAt);
//		}
//		Debug::DrawLine(objectPosition, objectForward * 100, Debug::RED);
//	}
//}
