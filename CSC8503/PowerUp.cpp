#include "PowerUp.h"
#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

PowerUp::PowerUp()
{
	tag = "PowerUp";
	PowerUpType = powerUpType::none;
	//setActivePowerup(powerUpType::none);
}

void PowerUp::OnTriggerBegin(GameObject* otherObject)
{

}
