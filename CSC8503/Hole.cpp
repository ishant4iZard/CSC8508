#include "Hole.h"
#include "Projectile.h"
#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"

using namespace NCL;
using namespace CSC8503;

Hole::Hole() {

}

void Hole::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Projectile") {
		Projectile* Bullet = dynamic_cast<Projectile*>(otherObject);
		Bullet->GetOwner()->AddScore(1);
		Bullet->deactivate();
		DeactivateProjectilePacket newPacket;
		newPacket.NetObjectID = Bullet->GetNetworkObject()->GetNetworkID();
		if (Bullet->GetGame()->GetServer())
		{
			Bullet->GetGame()->GetServer()->SendGlobalPacket(newPacket);
		}
	}
}