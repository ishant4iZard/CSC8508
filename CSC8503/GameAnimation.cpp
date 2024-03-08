#include "GameAnimation.h"
#include "MaleGuard.h"

using namespace NCL;
using namespace NCL::CSC8503;

GameAnimation::GameAnimation() {
	anmObjectsIDCounter = 0;
	anmObjectsStateCounter = 0;
}

void GameAnimation::Clear() {
	animatedObjects.clear();
	anmObjectsIDCounter = 0;
	anmObjectsStateCounter = 0;
}

void GameAnimation::AddAnimatedObject(MaleGuard* animatedObject) {
	animatedObjects.emplace_back(animatedObject);
	animatedObject->SetAnimatedObjectID(anmObjectsIDCounter++);
	anmObjectsStateCounter++;
}