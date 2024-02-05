#include "Projectile.h"
#include "NetworkedGame.h"
#include "NetworkPlayer.h"

using namespace NCL;
using namespace CSC8503;

int Projectile::CurrentAvailableProjectileID = 1000;

Projectile::Projectile(NetworkPlayer* Owner, NetworkedGame* Game)
{
	this->Owner = Owner;
	this->Game = Game;
	timeLeft = LIFE_TIME_AMOUNT;
}

Projectile::~Projectile()
{
}

void Projectile::OnCollisionBegin(GameObject* otherObject)
{
}
