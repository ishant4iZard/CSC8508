#include "PS5_Game.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"

#include "../PS5Core/PS5Window.h"

#include <iostream>

using namespace NCL;
using namespace CSC8503;

NCL::CSC8503::PS5_Game::PS5_Game()
{
	SpawnPlayer();
	StartLevel();
}

NCL::CSC8503::PS5_Game::~PS5_Game()
{
	delete player;
}

void NCL::CSC8503::PS5_Game::UpdateGame(float dt)
{
	physics->Update(dt);
	MovePlayer(dt);
	TutorialGame::UpdateGame(dt);
}

void NCL::CSC8503::PS5_Game::SpawnProjectile(NetworkPlayer* player, Vector3 firePos, Vector3 fireDir)
{
}

void NCL::CSC8503::PS5_Game::SpawnPlayer()
{
	float meshSize = 2.0f;
	float inverseMass = 1.0f / 600000.0f;

	player = new NetworkPlayer(this, 0);

	SphereVolume* volume = new SphereVolume(1.6f);

	player->SetBoundingVolume((CollisionVolume*)volume);
	player->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(Vector3(0, 10, -75));

	player->SetRenderObject(new RenderObject(&player->GetTransform(), charMesh, nullptr, basicShader));
	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));

	player->GetPhysicsObject()->SetInverseMass(inverseMass);
	player->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(player);
	player->GetRenderObject()->SetColour(Debug::RED);
}

void NCL::CSC8503::PS5_Game::StartLevel()
{
	physics->createStaticTree();
}

void NCL::CSC8503::PS5_Game::EndLevel()
{
}

void NCL::CSC8503::PS5_Game::MovePlayer(float dt) {
	float horizontalInput	= controller->GetNamedAxis("XLook");
	float verticalInput		= controller->GetNamedAxis("YLook");
	player->MovePlayerBasedOnController(dt, horizontalInput, verticalInput);
}