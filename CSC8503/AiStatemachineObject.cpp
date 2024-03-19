#include "AiStatemachineObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "TutorialGame.h"
#include "Projectile.h"
#include "PhysicsSystem.h"
#include "Debug.h"

using namespace NCL;
using namespace CSC8503;

Vector3 PredictInterceptionPoint(Vector3 p1, Vector3 v1, Vector3 p2, float s2) {
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
	this->settag("AI");

	State* PatrolState = new State([&](float dt) -> void
		{
			this->GetRenderObject()->SetColour(Debug::BLUE);
			currentState = PATROL;
			MoveRandomly(dt);
		}
	);
	State* ChaseState = new State([&](float dt) -> void
		{
			this->GetRenderObject()->SetColour(Debug::RED);
			currentState = CHASE;
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
			return (
				distanceToNearestProj >= DETECTION_RADIUS + 300 && 
				projectileToChase->GetPhysicsObject()->GetLinearVelocity().Length() > 5);
		}
	));

	randomMovementDirection = Vector3(rand() % 200, 5.6, rand() % 200) - transform.GetPosition();
	randomMovementDirection = randomMovementDirection.Normalised();
	projectileToChase = nullptr;
	isCollidingWithProj = false;
}

AiStatemachineObject::~AiStatemachineObject() {
	if (stateMachine) {
		delete stateMachine;
		stateMachine = NULL;
	}
}

void AiStatemachineObject::Update(float dt) {
	stateMachine->Update(dt);
	//if(navGrid) navGrid->PrintGrid();
	if (isCollidingWithProj && projectileToChase)
		projectileToChase->ReduceTimeLeft(dt * 5);

	//std::cout << transform.GetPosition().y << "\n";
}

void AiStatemachineObject::MoveRandomly(float dt) {
	randomMovementDirection.y = 0;
	Vector3 newVelocity = Vector3::Lerp(this->GetPhysicsObject()->GetLinearVelocity(), randomMovementDirection * SPEED, dt);
	newVelocity.y = 0;
	this->GetPhysicsObject()->SetLinearVelocity(newVelocity);

}

void AiStatemachineObject::DetectProjectiles(std::vector<Projectile*> ProjectileList) {
	Vector3 objectPosition = this->GetTransform().GetPosition();
	Vector3 objectForward = this->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	bool projFound = false;
	float shortestDistance = INT_MAX;

	for (auto proj : ProjectileList) {
		if (!proj->IsActive()) continue;

		Vector3 projPos = proj->GetTransform().GetPosition();
		float distance = (projPos - objectPosition).Length();

		if (distance < DETECTION_RADIUS && distance < shortestDistance) {
			projectileToChase = proj;
			shortestDistance = distance;
			distanceToNearestProj = distance;
			projFound = true;
		}
	}
	
	if (!projFound) distanceToNearestProj = INT_MAX;
}

void AiStatemachineObject::ChaseClosestProjectile(float dt) {
	Vector3 movementDirection;
	if (CanSeeProjectile()) {
		Vector3 targetPosition = projectileToChase->GetTransform().GetPosition();
		targetPosition = PredictInterceptionPoint(targetPosition, projectileToChase->GetPhysicsObject()->GetLinearVelocity(), transform.GetPosition(), SPEED);

		movementDirection = (targetPosition - this->GetTransform().GetPosition()).Normalised();
	}

	// Only use path finding if the projectile is not directly acccessible
	else {
		FindPathFromAIToProjectile(dt);
		//DisplayPathfinding();
		const static float bufferDistance = 0.2f;

		if (projectileToChase == nullptr || pathFromAIToPlayer.empty()) return;

		Vector3 targetPosition = pathFromAIToPlayer[0];
		Vector3 currentPosition = pathFromAIToPlayer[0];

		int index = 0;
		while ((index < pathFromAIToPlayer.size() - 1) &&
			((currentPosition - targetPosition).Length() < bufferDistance))
		{
			index++;
			targetPosition = pathFromAIToPlayer[index];
		}

		if (index == pathFromAIToPlayer.size() - 1)
			targetPosition = projectileToChase->GetTransform().GetPosition();


		movementDirection = (targetPosition - this->GetTransform().GetPosition()).Normalised();
	}
	movementDirection.y = 0;
	Vector3 newVelocity = Vector3::Lerp(this->GetPhysicsObject()->GetLinearVelocity(), randomMovementDirection * SPEED, dt);
	newVelocity.y = 0;

	this->GetPhysicsObject()->SetLinearVelocity(Vector3::Lerp(this->GetPhysicsObject()->GetLinearVelocity(), movementDirection * SPEED, dt));

	randomMovementDirection = movementDirection; // The ai should not rapidly change directions after state change
}

bool AiStatemachineObject::CanSeeProjectile() {
	if (projectileToChase == nullptr || !projectileToChase->IsActive()) return false;

	Vector3 dir = projectileToChase->GetTransform().GetPosition() - transform.GetPosition();
	dir = dir.Normalised();
	Ray ray = Ray(transform.GetPosition(), dir);

	RayCollision  closestCollision;
	if (world->Raycast(ray, closestCollision, true, this) && (GameObject*)(closestCollision.node) == projectileToChase) return true;

	return false;
}

void AiStatemachineObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Projectile") {
		isCollidingWithProj = true;
	}
	else if (otherObject->GetPhysicsObject()->GetInverseMass() == 0 && currentState == PATROL) {
		randomMovementDirection = Vector3(rand() % 200, 5.6, rand() % 200) - transform.GetPosition();
		randomMovementDirection = randomMovementDirection.Normalised();
		randomMovementDirection.y = 0;

		// The random direction should not lead to another collision with a static object
		// Therfore the ai fires a ray and calculates the distance to the nearest static object along the movement direction and checks if the distance is acceptable
		Ray ray = Ray(transform.GetPosition(), randomMovementDirection);
		RayCollision  closestCollision;
		while (true) {
			world->Raycast(ray, closestCollision, true, this); 
			float distance = (((GameObject*)(closestCollision.node))->GetTransform().GetPosition() - transform.GetPosition()).Length();

			if (distance > 5)
				break;

			randomMovementDirection = Vector3(rand() % 200, 5.6, rand() % 200) - transform.GetPosition();
			randomMovementDirection = randomMovementDirection.Normalised();
			randomMovementDirection.y = 0;
		}
	}
}

void AiStatemachineObject::OnCollisionEnd(GameObject* otherObject)
{
	if (otherObject->gettag() == "Projectile") {
		isCollidingWithProj = false;
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