#include "GravityWell.h"
#include "Projectile.h"
#include "GameObject.h"
#include "Vector3.h"
#include "PhysicsObject.h"
#include "Projectile.h"
#include "NetworkedGame.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include <iostream>

//TODO ifdef can be used here

using namespace NCL;
using namespace CSC8503;

GravityWell::GravityWell()
{
	GravityRange = 10.0f;
	GravityForce = 10000.0f;        
}

bool GravityWell::IsWithinRange(Projectile* projectile)	
{
	Maths::Vector3 relativePos = projectile->GetTransform().GetPosition() - this->GetTransform().GetPosition();
	if (relativePos.Length() <= GravityRange) {
		//std::cout << "inRange";
		return true;
	}
	return false;
}

void GravityWell::PullProjectilesWithinField(std::vector<Projectile*> projectileList)
{
	for (auto i : projectileList) {
		if (i->IsActive() && i!=NULL) {
			if(IsWithinRange(i)) {
				AddGravityForce(i);
			}
		}
	}
}

void GravityWell::AddGravityForce(Projectile* projectile)
{
	Maths::Vector3 relativePos = (projectile->GetTransform().GetPosition() - this->GetTransform().GetPosition());
	Vector3 dir = relativePos.Normalised();

	float gravForce = pow(1-(relativePos.Length() / GravityRange),2) * GravityForce;

	projectile->GetPhysicsObject()->AddForce(-dir * GravityForce);

}

void GravityWell::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Projectile") {
		Projectile* Bullet = dynamic_cast<Projectile*>(otherObject);
		//Bullet->GetOwner()->AddScore(1);
		Bullet->deactivate();
		DeactivateProjectilePacket newPacket;
		newPacket.NetObjectID = Bullet->GetNetworkObject()->GetNetworkID();
		NetworkedGame* tempGame = dynamic_cast<NetworkedGame*>(Bullet->GetGame());
		if (tempGame->GetServer())
		{
			tempGame->GetServer()->SendGlobalPacket(newPacket);
		}
	}
}
