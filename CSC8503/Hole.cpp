#include "Hole.h"
#include "Projectile.h"
#include "CollisionDetection.h"
#include "NetworkPlayer.h"

#ifdef _WIN32
#include "NetworkedGame.h"
#include "NetworkObject.h"
#include "GameServer.h"
#endif

using namespace NCL;
using namespace CSC8503;

Hole::Hole() {

}

void Hole::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Projectile") {
		//CollisionDetection::CollisionInfo info;
		//CollisionDetection::ObjectIntersection(this, otherObject, info);
		Projectile* Bullet = dynamic_cast<Projectile*>(otherObject);
		Bullet->GetOwner()->AddScore(1);
		Bullet->deactivate();

#ifdef _WIN32
		DeactivateProjectilePacket newPacket;
		newPacket.NetObjectID = Bullet->GetNetworkObject()->GetNetworkID();
		NetworkedGame* tempGame = dynamic_cast<NetworkedGame*>(Bullet->GetGame());
		if (tempGame->GetServer())
		{
			tempGame->GetServer()->SendGlobalPacket(newPacket);
		}
#endif
	}
}