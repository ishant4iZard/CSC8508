#include "bullet.h"
#include "NetworkPlayer.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8503;

int bullet::bulletID = 100;
std::vector<bullet*> bullet::bulletList;
std::vector<bullet*> bullet::bulletsDiscard;

bullet::bullet(NetworkPlayer* owner)
{
	this->owner = owner;
}

void bullet::OnCollisionBegin(GameObject* otherObject)
{
	if (dynamic_cast<NetworkPlayer*>(otherObject))
	{
		NetworkPlayer* hitPlayer = (NetworkPlayer*)otherObject;
		if (hitPlayer != owner)
		{
			if (hitPlayer->GetPlayerNum() > 3)
			{
				owner->addPlayerScore(bulletHitAI);
				return;
			}
			owner->addPlayerScore(bulletHitPlayer);
		}
	}
}

bullet::~bullet() {
}

void bullet::DestroySelf()
{
	owner->getGame()->SeverSendBulletDelPckt(bulletID);
	owner->getGame()->RemoveObjectFromWorld(this, false);
}

void Item::OnCollisionBegin(GameObject* otherObject)
{
	if (dynamic_cast<NetworkPlayer*>(otherObject))
	{
		NetworkPlayer* hitPlayer = (NetworkPlayer*)otherObject;
		if (hitPlayer->GetPlayerNum() > 3)
		{
			return;
		}
		func(hitPlayer, this);
	}
}