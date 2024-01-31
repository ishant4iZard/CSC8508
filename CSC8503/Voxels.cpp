#include "Voxels.h"
#include "TutorialGame.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "Player.h"

using namespace NCL::CSC8503;

Voxels::Voxels(const std::string& objectName)
{
	tag = "Voxel";
	name = objectName;
	worldID = -1;
	isActive = true;
	boundingVolume = nullptr;
	physicsObject = nullptr;
	renderObject = nullptr;
	networkObject = nullptr;
	isDestroyed = false;
}

void Voxels::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->gettag() == "Player") {
		Player* player = dynamic_cast<Player*>(otherObject);
		if (player->getDestroyVoxels()) {
			isActive = false;
			isDestroyed = true;
			//player->AddScore(0);
		}
	}
}