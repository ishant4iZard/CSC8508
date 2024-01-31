#include "Player.h"
#include "PhysicsObject.h"
#include "Ray.h"
#include"Window.h"

using namespace NCL::CSC8503;

Player::Player(GameWorld& game ,const std::string& objectName) : world(game)
{
	tag = "Player";
	name = objectName;
	worldID = -1;
	isActive = true;
	boundingVolume = nullptr;
	physicsObject = nullptr;
	renderObject = nullptr;
	networkObject = nullptr;
	currentHealth = maxHealth;
}

void Player::Update(float dt) {
	Ray groundRay = Ray(transform.GetPosition(), Vector3(0, -1.0f, 0));
	RayCollision groundCollisionData;

	if (world.Raycast(groundRay, groundCollisionData, true, this))
	{
		float distance = (groundCollisionData.collidedAt - transform.GetPosition()).Length();
		isOnGround = (distance <= 2.15f);
	}
	if (score > 5) {
		canJump = true;
	}
	if (score > 29) {
		destroyVoxels = true;
	}
	if (destroyVoxels) {
		Debug::Print("you can now destroy sand", Vector2(60, 90));
	}
	else {
		Debug::Print("Collect 30 to destroy sand", Vector2(49, 90));
	}


	std::string a = "Score:" + std::to_string(score);
	if (currentHealth > 0) {
		Debug::Print("Collect 75 and return to start to win", Vector2(29, 95));
		Debug::Print(a, Vector2(5, 5));
		Debug::Print("Health:" + std::to_string(currentHealth), Vector2(65, 5));
	}
	//add in tut
	if (currentHealth <= 0) {
		
	}

}