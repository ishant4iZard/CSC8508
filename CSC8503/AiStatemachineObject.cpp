#include "AiStatemachineObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "TutorialGame.h"
#include "Projectile.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "Debug.h"

using namespace NCL;
using namespace CSC8503;

AiStatemachineObject::AiStatemachineObject(GameWorld* world, NavigationGrid* navGrid) {
	stateMachine = new StateMachine();
	this->world = world;
	this->navGrid = navGrid;

	State* PatrolState = new State([&](float dt) -> void
		{
			DetectProjectiles(this, dt);
			MoveRandomly(dt);
		}
	);
	State* ChaseState = new State([&](float dt) -> void
		{
			DetectProjectiles(this, dt);
			ChaseClosestProjectile(dt);
		}
	);

	stateMachine->AddState(PatrolState);
	stateMachine->AddState(ChaseState);

	stateMachine->AddTransition(new StateTransition(PatrolState, ChaseState,
		[&]() -> bool
		{		
			if (distanceToNearestProj < DETECTION_RADIUS) return true;
			return false;
		}
	));

	stateMachine->AddTransition(new StateTransition(ChaseState, PatrolState,
		[&]() -> bool
		{
			if (distanceToNearestProj > DETECTION_RADIUS) return true;
			return false;
		}
	));
}

AiStatemachineObject::~AiStatemachineObject() {
	delete stateMachine;
}

void AiStatemachineObject::Update(float dt) {
	stateMachine->Update(dt);
}

void AiStatemachineObject::MoveRandomly(float dt) {
	// To Do : Add random movement
	GetPhysicsObject()->AddForce({ 0 , 0 , -10 });
	GetPhysicsObject()->SetLinearVelocity({ GetPhysicsObject()->GetLinearVelocity().x,0, GetPhysicsObject()->GetLinearVelocity().z });
}

void AiStatemachineObject::DetectProjectiles(GameObject* gameObject,float dt) {
	Vector3 objectPosition = gameObject->GetTransform().GetPosition();
	Vector3 objectForward = gameObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	const int numRays = 30;
	const float angleIncrement = 2 * 3.14 / numRays;
	vector<Ray> rays;
	for (int i = 0; i < numRays; i++) {
		float angle = angleIncrement * i;
		float x = cos(angle);
		float z = sin(angle);

		Vector3 dir = Vector3(x, 0, z);
		rays.push_back(Ray(objectPosition, dir));
		Debug::DrawLine(objectPosition, dir * 100, Debug::RED);
	}

	float shortestDistance = INT_MAX;
	bool projFound = false;
	RayCollision  closestCollision;
	for (auto ray : rays) {
		if (world->Raycast1(ray, closestCollision, true, gameObject)) {
			GameObject* ObjectHit = (GameObject*)closestCollision.node;

			if (ObjectHit != nullptr && ObjectHit->gettag() == "Projectile")
			{
				float distance = (ObjectHit->GetTransform().GetPosition() - objectPosition).Length();

				if (distance < shortestDistance) {
					projectileToChase = ObjectHit;
					shortestDistance = distance;
					distanceToNearestProj = distance;
					projFound = true;
				}
			}
		}
	}

	if (!projFound) distanceToNearestProj = INT_MAX;
}

void AiStatemachineObject::ChaseClosestProjectile(float dt) {
	const static int SPEED =3000;
	if (projectileToChase == nullptr) return;

	Vector3 movementDirection = (projectileToChase->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised();

	this->GetPhysicsObject()->SetLinearVelocity(movementDirection * SPEED * dt);
}

void AiStatemachineObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Projectile") {
		// Nothing to do here :-)
	}
}

