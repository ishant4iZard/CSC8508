#include "PowerUp.h"
#include "PhysicsObject.h"
#include "Player.h"

using namespace NCL::CSC8503;

Powerup::Powerup(const std::string& objectName)
{
	tag = "Powerup";
	name = objectName;
	worldID = -1;
	isActive = true;
	boundingVolume = nullptr;
	physicsObject = nullptr;
	renderObject = nullptr;
	networkObject = nullptr;
}

void Powerup::Update(float dt) {
	elaspedTime += dt;

	if (currentState == PowerUpState::Cooldown)
	{
		respawnTimeCurrent -= dt;
		if (respawnTimeCurrent <= 0.0f) ResetPowerup();
	}
	else if (currentState == PowerUpState::Active)
	{
		scale = transform.GetScale();
		Vector3 currentPos = transform.GetPosition();
		currentPos.y += 0.1*sin(elaspedTime * 5.0f) / 12.0f;
		transform.SetPosition(currentPos);
	}
}

void Powerup::OnTriggerBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Player" && currentState == PowerUpState::Active)
	{
		Player* player = dynamic_cast<Player*>(otherObject);
		if (player != nullptr)
		{
			player->AddScore(bonus);
			isActive = false;
		}
	}
}

void Powerup::ResetPowerup()
{
	respawnTimeCurrent = 0.0f;
	currentState = PowerUpState::Active;
	transform.SetScale(scale*2);
}