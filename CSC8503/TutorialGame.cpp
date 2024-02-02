#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "Voxels.h"
#include "Player.h"
#include "PowerUp.h"
#include "Goose.h"
#include "BouncePad.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include <Maths.h>


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

	forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");
	currentlevel = level::level1;

	gameover = false;
	player = NULL;


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
		player = NULL;
		world->ClearAndErase();
		physics->Clear();
		powerupList.clear();
		VoxelList.clear();
		EnemyList.clear();
		InitCamera();
		Debug::Print("GameOver", Vector2(40, 10), Debug::RED);
		Debug::Print("Final Score:" + std::to_string((int)(score)), Vector2(40, 30));
		Debug::Print("Press 'F1' to restart", Vector2(40, 50));
		Debug::Print("or", Vector2(40, 60));
		Debug::Print("Press 'ESC' to exit", Vector2(40, 70));
	}
	else if(gameWon)
	{
		player = NULL;
		world->ClearAndErase();
		physics->Clear();
		powerupList.clear();
		VoxelList.clear();
		EnemyList.clear();
		InitCamera();
		Debug::Print("You Won!!!", Vector2(40, 10), Debug::GREEN);
		Debug::Print("You saved the world from goats!!!", Vector2(40, 20), Debug::GREEN);
		Debug::Print("Final Score:" + std::to_string((int)(score+(300-finaltimer)/10)), Vector2(40, 30));
		Debug::Print("Finish Time:" + std::to_string((int)(finaltimer)), Vector2(40, 40));
		Debug::Print("Press 'F1' to restart", Vector2(40, 50));
		Debug::Print("or", Vector2(40, 60));
		Debug::Print("Press 'ESC' to exit", Vector2(40, 70));
	}

	else {
		timer += dt;
		if (!inSelectionMode) {
			world->GetMainCamera().UpdateCamera(dt);
		}

		for (size_t i = 0; i < powerupList.size(); i++)
			powerupList[i]->Update(dt);

		for (size_t i = 0; i < EnemyList.size(); i++)
			EnemyList[i]->Update(dt);

		for (size_t i = 0; i < Enemy2List.size(); i++)
			Enemy2List[i]->Update(dt);

		Debug::Print("Time left:" + std::to_string((int)(300-timer)), Vector2(5, 10));

		if (player) {
			player->Update(dt);
			if (player->GetHealth() <= 0 || timer > 300.0f) {
				gameover = true;
				score = player->GetScore();
			}
			if (player->GetScore() > 25 && player->GetTransform().GetPosition().x < -10 && player->GetTransform().GetPosition().z>0) {
				gameWon = true;
				score = player->GetScore();
				finaltimer = timer;
			}
		}

		//UpdateVoxels();


		/*if (useGravity) {
			Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
		}
		else {
			Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
		}*/

		if (testStateObject) {
			testStateObject->Update(dt);
		}

		/*RayCollision closestCollision;
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
			Vector3 rayPos;
			Vector3 rayDir;

			rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

			rayPos = selectionObject->GetTransform().GetPosition();

			Ray r = Ray(rayPos, rayDir);

			if (world->Raycast(r, closestCollision, true, selectionObject)) {
				if (objClosest) {
					objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				}
				objClosest = (GameObject*)closestCollision.node;

				objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
			}
		}*/

		//Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 1, 1));

		SelectObject();
		MoveSelectedObject();

		world->UpdateWorld(dt);
		renderer->Update(dt);
		//physics->Update(dt);
	}

	UpdateKeys();

	renderer->Render();

	Debug::UpdateRenderables(dt);
	
	
	
}

void TutorialGame::UpdateVoxels() {
	for (int it = 0; it < VoxelList.size(); it++) {
		int k = it % 20;
		int j = (it / 20) % 3;
		int i = (it / 60) % 20;
		if (i==0) {
			if (VoxelList[it+60]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}
		else if (i == 19) {
			if (!VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}
		else {
			if (VoxelList[it + 60]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
			if (VoxelList[it - 60]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}
		if (j==0) {
			if (VoxelList[it + 20]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}
		else if (j == 2) {
			if (!VoxelList[it]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}
		else {
			if (VoxelList[it + 20]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
			if (VoxelList[it - 20]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}
		if (k==0) {
			if (VoxelList[it+1]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}
		else if (k == 19) {
			if (!VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}
		else {
			if (VoxelList[it+1]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
			if (VoxelList[it-1]->Destroyed() && !VoxelList[it]->Destroyed()) {
				VoxelList[it]->activate();
				continue;
			}
		}

	}
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	//if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
	//	InitCamera(); //F2 will reset the camera to a specific default place
	//}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		InitWorld2(); 
	}

	//if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
	//	useGravity = !useGravity; //Toggle gravity!
	//	physics->UseGravity(useGravity);
	//}
	/*if (Window::GetKeyboard()->KeyPressed(KeyCodes::U)) {
		switch (currentlevel)
		{
		case level::level1: {
			currentlevel = level::level2;
			break;
		}
		case level::level2: {
			currentlevel = level::level1;
			break;
		}
		default:
			break;
		}
		InitWorld();
	}*/
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	/*if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}*/

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!
	Quaternion orientation = lockedObject->GetTransform().GetOrientation();

	Vector3 upAxis = Vector3(0, 1, 0);
	Vector3 fwdAxis = Vector3::Cross( upAxis,rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	v-= (Window::GetMouse()->GetRelativePosition().y);
	v = std::clamp(v, -45.0f, 45.0f);

	h -= (Window::GetMouse()->GetRelativePosition().x);
	if (h < 0) h += 360.0f;
	if (h > 360.0f) h -= 360.0f;

	Matrix4 yawMat = Matrix4::Rotation(h, upAxis);
	Matrix4 pitchMat = Matrix4::Rotation(v, yawMat * Vector3(-1, 0, 0));
	Matrix4 finalRotMat = pitchMat * yawMat;

	Vector3 focusPoint = lockedObject->GetTransform().GetPosition();
	Vector3 lookDirection = finalRotMat * Vector3(0, 0, -1);

	Vector3 position = focusPoint - lookDirection * 6;

	Ray collisionRay = Ray(focusPoint, -lookDirection);
	RayCollision collisionRayData;
	if (world->Raycast(collisionRay, collisionRayData, true, lockedObject))
	{
		if (collisionRayData.rayDistance < 6)
			position = focusPoint - lookDirection * (collisionRayData.rayDistance - 1.0f);
	}

	Vector3 force = Vector3();

	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		force += Vector3(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		force += Vector3(-fwdAxis);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		force += Vector3(-rightAxis);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		force += Vector3(rightAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE) && player->grounded() &&player->Jump()) {
		Vector3 velocity = lockedObject->GetPhysicsObject()->GetLinearVelocity();
		lockedObject->GetPhysicsObject()->SetLinearVelocity(Vector3(velocity.x,8,velocity.z));
	}

	Vector3 velocity = lockedObject->GetPhysicsObject()->GetLinearVelocity();
	float Hspeed = Vector2(velocity.x, velocity.z).Length();
	float Vspeed = velocity.y;
	//if(Hspeed<8.0f)
	lockedObject->GetPhysicsObject()->AddForce(force*8);

	if (Vspeed < -10) {
		player->TakeDamage(500);
	}

	Matrix4 viewMatrix = Matrix4::BuildViewMatrix(position, lockedObject->GetTransform().GetPosition(), Vector3(0, 1, 0)).Inverse();
	Quaternion q(viewMatrix);
	float pitch = q.ToEuler().x + 10.0f;
	float yaw = q.ToEuler().y;

	Quaternion lookat = Quaternion::EulerAnglesToQuaternion(0,yaw,0);

	lockedObject->GetTransform().SetOrientation(lookat);

	world->GetMainCamera().SetPosition(position)
		.SetPitch(pitch)
		.SetYaw(yaw);

	/*if (player->Jump()) {
		Debug::Print("Press Space to jump", Vector2(60, 85));
	}*//*
	else {
		Debug::Print("collect 6 points to enable jump", Vector2(40, 85));
	}*/

	/*if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}*/

}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	//if (inSelectionMode && selectionObject) {
	//	//Twist the selected object!
	//	if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
	//		selectionObject->GetPhysicsObject()->AddForce(Vector3(-10, 0, 0));
	//	}

	//	if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
	//		selectionObject->GetPhysicsObject()->AddForce(Vector3(10, 0, 0));
	//	}

	//	if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
	//		selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
	//	}

	//	if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
	//		selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
	//	}

	//	/*if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
	//		selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
	//	}*/

	//	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
	//		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
	//	}

	//	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
	//		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
	//	}

	//	if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
	//		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	//	}
	//	if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM6)) {
	//		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 10, 0));
	//	}
	//}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-90.0f);
	world->GetMainCamera().SetYaw(0.0f);
	world->GetMainCamera().SetPosition(Vector3(0, 245, 0));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	gameover = false;
	world->ClearAndErase();
	physics->Clear();
	powerupList.clear(); 
	//VoxelList.clear();
	EnemyList.clear();
	Enemy2List.clear();
	InitCamera();
	physics->UseGravity(true);
	physics->SetBroadphase(true);
	timer = 0;

	/*switch (currentlevel)
	{
		case level::level1: {
			
			break;
		}
		case level::level2: {
			
			break;
		}
	}*/
	//InitCubeGridWorld(20, 20, 3, 2, 2, Vector3(1, 1, 1));
	//InitCoin(20, 1);
	//AddEnemyToWorld(Vector3(-25, -10, 0));
	InitDefaultFloor();
	InitBouncePad();
	//InitBoundary();
	//BridgeConstraintTest();
	//InitMaze();
	//AddPlayerToWorld(Vector3(61, -16, 28));
	//InitAI();
	selectionObject = NULL;

	//InitOBBwall();
	//AddStateObjectToWorld(Vector3(-25, -16, 10));
	
	//InitMaze();
	//physics->createStaticTree();
}

void TutorialGame::InitWorld2() {
	gameover = false;
	world->ClearAndErase();
	physics->Clear();
	powerupList.clear();
	VoxelList.clear();
	EnemyList.clear();
	Enemy2List.clear();
	player = NULL;
	physics->SetBroadphase(false);
	selectionObject = NULL;
	timer = 0;

	InitCamera();

	physics->UseGravity(false);

	InitOBBwall();
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
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass , bool isHollow , float elasticity) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia(isHollow);
	sphere->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity) {
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
GameObject* TutorialGame::AddVoxelsToWorld(const Vector3& position, Vector3 dimensions, float elasticity) {
	Voxels* cube = new Voxels("voxels");

	AABBVolume* volume = new AABBVolume(dimensions,false,true);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, sandTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetElasticity(elasticity);


	VoxelList.push_back(cube);

	world->AddGameObject(cube);

	cube->deactivate();


	return cube;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float radius, float halfHeight, float inverseMass, float elasticity) {
	GameObject* capsule = new GameObject("capsule");

	CapsuleVolume* volume = new CapsuleVolume(halfHeight,radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);
	Vector3 capsuleSize = Vector3(2*radius, 2*halfHeight , 2*radius);

	capsule->GetTransform()
		.SetPosition(position)
		.SetScale(capsuleSize);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();
	capsule->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(capsule);

	return capsule;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 1.0f;
	float inverseMass	= 0.5f;

	Goose* character = new Goose(*world, player,"Goose" );
	SphereVolume* volume  = new SphereVolume(1.2f); 
	character->SetBoundingVolume((CollisionVolume*)volume);
	
	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), gooseMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	world->AddGameObject(character);

	EnemyList.push_back(character);

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 2.0f;
	float inverseMass	= 0.5f;

	Player* character = new Player(*world, "Player");

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.7f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	character->GetRenderObject()->SetColour(Vector4(0.0f, 150.0f, 240.0f, 255.0f) / 255.0f);

	character->settag("Player");
	character->setDestroyVoxels(false);

	player = character;

	world->AddGameObject(character);
	//lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

Powerup* TutorialGame::AddScoreToWorld(const Vector3& position, float radius, float inverseMass , int bonusScore) {
	Powerup* sphere = new Powerup("coin");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius, true, true);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(0);
	sphere->GetPhysicsObject()->InitSphereInertia(false);
	sphere->GetPhysicsObject()->SetElasticity(0.7f);

	sphere->setBonus(bonusScore);

	sphere->GetRenderObject()->SetColour(Vector4(1, 0.87f, 0, 1));

	powerupList.push_back(sphere);

	world->AddGameObject(sphere);

	return sphere;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(2.0f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), charMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(5.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	Enemy2List.push_back(apple);

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitAI() {
	for (int x = 0; x < 4; ++x) {
		Vector3 position = Vector3((rand() % 128) - 128, -15, (rand() % 128) - 64);
		AddStateObjectToWorld(position);
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0), Vector3(100, 2, 100));
	/*AddFloorToWorld(Vector3(0, -20, 0));
	AddFloorToWorld(Vector3(32, -20, -96), Vector3(28, 2, 28));
	AddFloorToWorld(Vector3(96, -20, -86), Vector3(28, 2, 38));
	AddFloorToWorld(Vector3(96, -18, -28), Vector3(28, 2, 28));
	AddFloorToWorld(Vector3(32, -20, 96), Vector3(28, 2, 28));
	AddFloorToWorld(Vector3(96, -20, 86), Vector3(28, 2, 38));
	AddFloorToWorld(Vector3(96, -18, 28), Vector3(28, 2, 28));*/
}

void TutorialGame::InitGameExamples() {
	//AddPlayerToWorld(Vector3(-30, 5, 0));
	AddEnemyToWorld(Vector3(-20, 5, 0));
	//AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitCoin(int Amount, float radius) {
	for (int x = 0; x < 6; ++x) {
		Vector3 position = Vector3((rand()%128)-128, -15, (rand() % 128) - 64);
		AddScoreToWorld(position, radius, 1.0f);
	}
	for (int x = 0; x < 5; ++x) {
		Vector3 position = Vector3((rand()%25)-110, -15, (rand() % 25) - 110 );
		AddScoreToWorld(position, radius, 1.0f , 5);
	}
	for (int x = 0; x < 2; ++x) {
		Vector3 position = Vector3((rand() % 25) + 32, (rand() % 3) -15, (rand() % 25) - 96);
		AddScoreToWorld(position, radius, 1.0f, 2);
	}
	for (int x = 0; x < 2; ++x) {
		Vector3 position = Vector3((rand() % 25) + 96, (rand() % 3) -15, (rand() % 25) - 86);
		AddScoreToWorld(position, radius, 1.0f, 2);
	}
	for (int x = 0; x < 2; ++x) {
		Vector3 position = Vector3((rand() % 25) + 32, - 15, (rand() % 25) + 96);
		AddScoreToWorld(position, radius, 1.0f, 2);
	}
	for (int x = 0; x < 2; ++x) {
		Vector3 position = Vector3((rand() % 25) + 92, - 15, (rand() % 25) + 86);
		AddScoreToWorld(position, radius, 1.0f, 2);
	}

	AddScoreToWorld(Vector3(96,-24,0), radius, 1.0f, 8);

	AddScoreToWorld(Vector3(38, -15, -20), radius, 1.0f, 20);
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	float capsuleRadius = 1.0f;
	float halfHeight = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			int random = rand() % 3;
			if (random == 0) {
				AddCubeToWorld(position, cubeDims);
			}
			else if (random ==1) {
				AddCapsuleToWorld(position, capsuleRadius, halfHeight);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitBoundary() {
	InitInvisibleWall(Vector3(128, -9, 0), Vector3(2, 10, 128));
	InitInvisibleWall(Vector3(-128, -9, 0), Vector3(2, 10, 128));
	InitInvisibleWall(Vector3(0, -9, 128), Vector3(128, 10, 2));
	InitInvisibleWall(Vector3(0, -9, -128), Vector3(128, 10, 2));
}

void TutorialGame::InitInvisibleWall(Vector3 position,Vector3 size) {
	GameObject* wall = new GameObject();

	AABBVolume* volume = new AABBVolume(size);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform()
		.SetScale(size * 2)
		.SetPosition(position);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(wall);
}

void TutorialGame::InitCubeGridWorld(int length, int breadth , int height, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 0; x < length; x++) {
		for (int y = 0; y < height; y++) {
			for (int z = 0; z < breadth; z++) {
				Vector3 position = Vector3(-126 + (x * colSpacing), -17.0f + y * 2, -126 + z * rowSpacing);
				//AddVoxelsToWorld(position, cubeDims, 1.0f);

			}
		}
	}
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(6, 1, 2);
	float invCubeMass = 0.01; // how heavy the middle pieces are
	int numLinks = 21;
	float maxDistance = 4.3; // constraint distance
	float cubeDistance = 4; // distance between links

	Vector3 startPos = Vector3(96, -18, -46);
	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0)
		, cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3(0, 0, (numLinks + 2)
		* cubeDistance), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3(0, 0, (i + 1) *
			cubeDistance), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous,
			block, maxDistance);
		world -> AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous,
		end, maxDistance);
	world->AddConstraint(constraint);

}
void TutorialGame::InitBouncePad()
{
	for (size_t i = 0; i < 1; i++)
	{
		BouncePad* tempBouncePad = new BouncePad(cubeMesh, basicTex, basicShader);
		bouncePadList[i] = tempBouncePad;
		world->AddGameObject(bouncePadList[i]);
	}
}

void TutorialGame::InitLevelWall()
{

}

void TutorialGame::InitOBBwall() {
	AddObbCubeToWorld(Vector3(-111, -19, 111), Vector3(10, 2, 10), 0);
	AddObbCubeToWorld(Vector3(-111, -14, 111), Vector3(1, 1, 1));
	AddObbCubeToWorld(Vector3(-111, -10, 111), Vector3(1, 1, 1));
	AddObbCubeToWorld(Vector3(-111, -8, 111), Vector3(1, 1, 1));
	AddObbCubeToWorld(Vector3(-107, -14, 111), Vector3(1, 1, 1));
	AddObbCubeToWorld(Vector3(-107, -10, 111), Vector3(1, 1, 1));
	AddObbCubeToWorld(Vector3(-107, -8, 111), Vector3(1, 1, 1));
	AddObbCubeToWorld(Vector3(-115, -14, 111), Vector3(1, 1, 1));
	AddObbCubeToWorld(Vector3(-115, -10, 111), Vector3(1, 1, 1));
	AddObbCubeToWorld(Vector3(-115, -8, 111), Vector3(1, 1, 1));
}


/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		//Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());


			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				//Debug::DrawLine(ray.GetPosition(), closestCollision.collidedAt, { 1,0,0,1 },1000);
				return true;
			}
			else {
				return false;
			}
		}
		/*if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}*/
	}
	else {
		//Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	//Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

//level initialtion
void TutorialGame::InitMaze() {
	int y = 0;
	int scale = 3;
	float wallThickness = 1.0f;
	Vector3 posShift = { 0,0,35 };

	AddFloorToWorld	(Vector3	 (35, -20+y,0), Vector3(35, 2, 35));
	InitInvisibleWall(Vector3	(34, -9.0f+y, 5.5f	-35), Vector3(31.25f, 18.0f, wallThickness));
	InitInvisibleWall(Vector3	(34, -9.0f+y, 65.5f	-35), Vector3(31.25f, 18.0f, wallThickness));
	InitInvisibleWall(Vector3	(63.25f, -9.0f+y, 0), Vector3(wallThickness, 18.0f, 31.25f));
	InitInvisibleWall(Vector3	(5.0, -9.0f + y, 38.5f	-35), Vector3(wallThickness, 18.0f, 27.0f));
	InitInvisibleWall(Vector3	(20.75, -9.0f + y, 49.5	-35), Vector3(wallThickness, 18.0f, 15.0f)) ;
	InitInvisibleWall(Vector3	(49.5, -9.0f + y, 20.75	-35), Vector3(wallThickness, 18.0f, 15.0f)) ;
	InitInvisibleWall(Vector3	(35.5, -9.0f + y, 50.76	-35), Vector3(wallThickness, 18.0f, 10.0f) ) ;
	InitInvisibleWall(Vector3	(35.5, -9.0f + y, 14.5f	-35), Vector3(wallThickness, 18.0f, 5.0f) ) ;
	InitInvisibleWall(Vector3	(12.0, -9.0f + y, 13.5f	-35), Vector3(5.5f, 18.0f, wallThickness)) ;
	InitInvisibleWall(Vector3	(59.75, -9.0f + y, 57.5f-35), Vector3(5.5f, 18.0f, wallThickness)) ;
	InitInvisibleWall(Vector3	(19.5, -9.0f + y, 35.5f	-35), Vector3(7.25f, 18.0f, wallThickness)) ;
	InitInvisibleWall(Vector3	(53.25, -9.0f + y, 35.5f-35), Vector3(5.5f, 18.0f, wallThickness)  ) ;
	InitInvisibleWall(Vector3	(35.5, -9.0f + y, 52.0f	-35), Vector3(5.5f, 18.0f, wallThickness) ) ;
	InitInvisibleWall(Vector3	(35.5, -9.0f + y, 19.5f	-35), Vector3(5.5f, 18.0f, wallThickness) ) ;
	InitInvisibleWall(Vector3	(46.5, -9.0f + y, 57.0f	-35), Vector3(wallThickness, 18.0f, 9.0f) ) ;
	InitInvisibleWall(Vector3	(23.5, -9.0f + y, 15.75f-35), Vector3(wallThickness, 18.0f, 9.0f) ) ;
	InitInvisibleWall(Vector3	(40.75, -9.0f + y, 27.5f-35), Vector3(9.0f, 18.0f, wallThickness) ) ;
	InitInvisibleWall(Vector3	(57.0, -9.0f + y, 41.5f	-35), Vector3(wallThickness, 18.0f, 4.5f) ) ;
	InitInvisibleWall(Vector3	(20.0, -9.0f + y, 23.75f-35), Vector3(4.5f, 18.0f, wallThickness) ) ;


	//InitInvisibleWall(Vector3	(5, -1, -28), Vector3(1, 1, 1) ) ;
	//InitInvisibleWall(Vector3	(61, -1, 28), Vector3(1, 1, 1) ) ;


}

