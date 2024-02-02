#include "Hole.h"
#include "Projectile.h"
#include "NetworkPlayer.h"

using namespace NCL;
using namespace CSC8503;

Hole::Hole() {

}

void Hole::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Projectile") {
		Projectile* Bullet = dynamic_cast<Projectile*>(otherObject);
		Bullet->GetOwner()->AddScore(1);
		Bullet->deactivate();
	}
}