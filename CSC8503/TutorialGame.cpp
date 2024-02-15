#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "Hole.h"
#include "BouncePad.h"

#include <Maths.h>


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
	InitHole();
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
	delete aitreetest;
}

void TutorialGame::UpdateGame(float dt) {
	if (gameover) {
		world->ClearAndErase();
		physics->Clear();
		InitCamera();
		Debug::Print("GameOver", Vector2(40, 10), Debug::RED);
		Debug::Print("Final Score:" + std::to_string((int)(score)), Vector2(40, 30));
		Debug::Print("Press 'ESC' to exit", Vector2(40, 50));
	}
	else if (gameWon)
	{
		world->ClearAndErase();
		physics->Clear();
		InitCamera();
		Debug::Print("You Won!!!", Vector2(40, 10), Debug::GREEN);
		Debug::Print("You saved the world from goats!!!", Vector2(40, 20), Debug::GREEN);
		Debug::Print("Final Score:" + std::to_string((int)(score + (300 - finaltimer) / 10)), Vector2(40, 30));
		Debug::Print("Finish Time:" + std::to_string((int)(finaltimer)), Vector2(40, 40));
		
		Debug::Print("Press 'ESC' to exit", Vector2(40, 50));
	}

	else if(serverStarted) {
		timer += dt;

		world->GetMainCamera().UpdateCamera(dt);

		if (timer > TIME_LIMIT) {
			gameover = true;

		}

		Debug::UpdateRenderables(dt);

		Debug::Print("Time left:" + std::to_string((int)(TIME_LIMIT-timer)), Vector2(5, 10));
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::C)) {
		//claptrap->OnAwake(2);
		aitreetest->ResetBehaviourTree();
		aitreetest->OnBehaviour();
	}

	aitreetest-> Update(dt);
	
	world->UpdateWorld(dt);
	renderer->Render();
	renderer->Update(dt);


	static int frameCounter = 0;
	//aitreetest->GetTransform().RandomPosition(aitreetest->GetTransform().GetPosition(), true).SetScale(Vector3(10, 10, 10) * 2);
	frameCounter++;

	if (frameCounter >= 100) {
		aitreetest->GetTransform().RandomPosition(aitreetest->GetTransform().GetPosition(), true);
		Vector3 aichaseposition = aitreetest->GetTransform().GetPosition();
		frameAddresses.push_back(aichaseposition);
		
		ProcessFrameAddresses();
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
	//AddAiToWorld(Vector3(0, -2, 0), Vector3(10, 10, 10), 1.0, 1.0);
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

AiTreeObject* TutorialGame::AddAiToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity) {
	aitreetest = new AiTreeObject("Aitreeobject");

	OBBVolume* volume = new OBBVolume(dimensions);
	aitreetest->SetBoundingVolume((CollisionVolume*)volume);

	//aitreetest->GetTransform().RandomPosition(position, true).SetScale(dimensions * 2);
		//.SetPosition(position).SetScale(dimensions * 2);

	aitreetest->SetRenderObject(new RenderObject(&aitreetest->GetTransform(), cubeMesh, basicTex, basicShader));
	aitreetest->SetPhysicsObject(new PhysicsObject(&aitreetest->GetTransform(), aitreetest->GetBoundingVolume()));

	aitreetest->GetPhysicsObject()->SetInverseMass(inverseMass);
	aitreetest->GetPhysicsObject()->InitCubeInertia();
	aitreetest->GetPhysicsObject()->SetElasticity(elasticity);
	aitreetest->BehaviorTree();

	world->AddGameObject(aitreetest);
	return aitreetest;

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
	AddObbCubeToWorld(Vector3(96,0,0), Vector3(10, 20, 100), 0, 0.5f);
	AddObbCubeToWorld(Vector3(-96, 0, 0), Vector3(10, 20, 100), 0, 0.5f);
	AddObbCubeToWorld(Vector3(0,0, 96), Vector3(100, 20, 10), 0, 0.5f);
	AddObbCubeToWorld(Vector3(0, 0, -96), Vector3(100, 20, 10), 0, 0.5f);
}

void TutorialGame::ProcessFrameAddresses() {

	for (auto address : frameAddresses) {

		std::cout << "Address: " << address << std::endl;
	}
	//frameAddresses.clear();
}
