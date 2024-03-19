#include "GameAnimation.h"
#include "MaleGuard.h"
#include "MaxGuard.h"
#include "GameObject.h"

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

void GameAnimation::AddAnimatedObject(MaxGuard* animatedObject) {
	animatedObjects.emplace_back(animatedObject);
	animatedObject->SetAnimatedObjectID(anmObjectsIDCounter++);
	anmObjectsStateCounter++;
}

void GameAnimation::AddAnimatedObject(GameObject* animatedObject) {
	animatedObjects.emplace_back(animatedObject);
	animatedObject->SetAnimatedObjectID(anmObjectsIDCounter++);
	anmObjectsStateCounter++;
}