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

Vector3 PredictInterceptionPoint(Vector3 p1, Vector3 v1, Vector3 p2, float s2) {
	return p1;

	// This makes the game lag for some reason
	Vector3 relativePositions = p1 - p2;

	Vector3 v2 = relativePositions.Normalised() * s2;
	float relativeSpeed = (v1 - v2).Length();

	if (relativeSpeed <= 0)	return p1;

	float timeToIntercept = relativePositions.Length() / relativeSpeed;
	Vector3 interceptionPoint = p1 + v1 * timeToIntercept;

	return interceptionPoint;
}

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
			FindPathFromAIToProjectile(dt);
			DisplayPathfinding();
			ChaseClosestProjectile(dt);
		}
	);

	stateMachine->AddState(PatrolState);
	stateMachine->AddState(ChaseState);

	stateMachine->AddTransition(new StateTransition(PatrolState, ChaseState,
		[&]() -> bool
		{		
			return (distanceToNearestProj < DETECTION_RADIUS);
		}
	));

	stateMachine->AddTransition(new StateTransition(ChaseState, PatrolState,
		[&]() -> bool
		{
			return (distanceToNearestProj >= DETECTION_RADIUS);
		}
	));
}

AiStatemachineObject::~AiStatemachineObject() {
	delete stateMachine;
}

void AiStatemachineObject::Update(float dt) {
	stateMachine->Update(dt);
	if(navGrid) navGrid->PrintGrid();
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
		//Debug::DrawLine(objectPosition, dir * 100, Debug::RED);
	}

	float shortestDistance = INT_MAX;
	bool projFound = false;
	RayCollision  closestCollision;
	for (auto ray : rays) {
		if (world->FindObjectByRaycast(ray, closestCollision, "Projectile", gameObject)) {
			GameObject* ObjectHit = (GameObject*)closestCollision.node;
			float distance = (ObjectHit->GetTransform().GetPosition() - objectPosition).Length();

			if (distance < shortestDistance) {
				projectileToChase = ObjectHit;
				shortestDistance = distance;
				distanceToNearestProj = distance;
				projFound = true;
			}
		}
	}

	if (!projFound) distanceToNearestProj = INT_MAX;
}

void AiStatemachineObject::ChaseClosestProjectile(float dt) {
	const static float bufferDistance = 0.2f;
	
	if (projectileToChase == nullptr || pathFromAIToPlayer.empty()) return;

	Vector3 targetPosition = pathFromAIToPlayer[0];
	Vector3 currentPosition = pathFromAIToPlayer[0];

	int index = 0;
	while   ((index < pathFromAIToPlayer.size() - 1) && 
			((currentPosition - targetPosition).Length() < bufferDistance))
	{
		index++;
		targetPosition = pathFromAIToPlayer[index];
	}
	
	if (index == pathFromAIToPlayer.size() - 1 )
		targetPosition = projectileToChase->GetTransform().GetPosition();

	targetPosition.y = 5.6f;

	Vector3 movementDirection = (targetPosition - this->GetTransform().GetPosition()).Normalised();
	movementDirection.y = 0;
	this->GetPhysicsObject()->SetLinearVelocity(movementDirection * SPEED);
}

void AiStatemachineObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Projectile") {
		// Nothing to do here :-)
	}
}

void AiStatemachineObject::FindPathFromAIToProjectile(float dt)
{
	pathFromAIToPlayer.clear();

	Vector3	AIPos		= transform.GetPosition(),
			targetPos	= projectileToChase->GetTransform().GetPosition();


	//targetPos = targetPos + projectileToChase->GetPhysicsObject()->GetLinearVelocity() * 5 * dt;
	targetPos = PredictInterceptionPoint(targetPos, projectileToChase->GetPhysicsObject()->GetLinearVelocity(), AIPos, SPEED);

	AIPos.y = targetPos.y = 0; // Only for testing, will remove later

	NavigationPath outPath;
	bool found = navGrid->FindPath(AIPos, targetPos, outPath);

	if (!found) {
		std::cout << "Path not found\n";
		return;
	}

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		pathFromAIToPlayer.push_back(pos);
	}
}

void AiStatemachineObject::DisplayPathfinding() {
	if (pathFromAIToPlayer.empty()) return;

	for (int i = 1; i < pathFromAIToPlayer.size(); ++i) {
		Vector3 a = pathFromAIToPlayer[i - 1];
		Vector3 b = pathFromAIToPlayer[i];
		Debug::DrawLine(a, b, Debug::GREEN);
	}
}