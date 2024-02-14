#include "Teleporter.h"
#include "Projectile.h"
#include "PhysicsObject.h"



NCL::CSC8503::Teleporter::Teleporter()
{
	connectedTeleporter == NULL;
	outNormal = this->GetTransform().GetOrientation().ToEuler().Normalised();
}

NCL::CSC8503::Teleporter::Teleporter(Teleporter* otherTeleporter)
{
	connectedTeleporter = otherTeleporter;
	outNormal = this->GetTransform().GetOrientation().ToEuler().Normalised();
	otherTeleporter->setConnectedTeleporter(this);
}

void NCL::CSC8503::Teleporter::OnTriggerBegin(GameObject* otherObject)
{
	if (otherObject->gettag() == "Projectile") {
		Projectile* Bullet = dynamic_cast<Projectile*>(otherObject);
		if (Bullet->getTeleport() && connectedTeleporter!=NULL) {
			Vector3 outNormal = connectedTeleporter->GetTransform().GetOrientation() * Vector3(0, 0, 1);
			Bullet->GetTransform().SetPosition(connectedTeleporter->GetTransform().GetPosition() + outNormal * 5 );
			float speed = Bullet->GetPhysicsObject()->GetLinearVelocity().Length();
			Bullet->GetPhysicsObject()->SetLinearVelocity(outNormal * speed);
			Bullet->setTeleport(false);
		}
	}
}

void NCL::CSC8503::Teleporter::OnTriggerEnd(GameObject* otherObject)
{
	if (otherObject->gettag() == "Projectile") {
		Projectile* Bullet = dynamic_cast<Projectile*>(otherObject);
		Bullet->setTeleport(true);
	}
}