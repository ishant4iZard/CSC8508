#include "AiStatemachineObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "TutorialGame.h"
#include "Debug.h"

using namespace NCL;
using namespace CSC8503;

AiStatemachineObject::AiStatemachineObject(GameWorld* world) {
	counter = 0.0f;
	stateMachine = new StateMachine();

	this->world = world;
	State* stateA = new State([&](float dt) -> void
		{
			this->MoveRound(dt);
		}
	);
	State* stateB = new State([&](float dt) -> void
		{
			this->ObjectDetectRay(this);
			this->Chasethebullets(0.2f);
			
		}
	);


	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB,
		[&]() -> bool
		{
			return this->counter > 5.0f;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateB, stateA,
		[&]() -> bool
		{
			///GetPhysicsObject()->SetLinearVelocity({ 0,0,0 });
			return this->counter < 0.0f;
		}
	));


}

AiStatemachineObject::~AiStatemachineObject() {
	delete stateMachine;
}

void AiStatemachineObject::Update(float dt) {
	stateMachine->Update(dt);
}


void AiStatemachineObject::MoveRound(float dt) {
	GetPhysicsObject()->AddForce({ 0 , 0 , -10 });
	GetPhysicsObject()->SetLinearVelocity({ GetPhysicsObject()->GetLinearVelocity().x,0, GetPhysicsObject()->GetLinearVelocity().z });
	//if(closestCollision.rayDistance < 10.0f)
	counter += dt;
}

void AiStatemachineObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 0 , 0 , 0 });
	//GetPhysicsObject()->SetLinearVelocity({ 0,0, GetPhysicsObject()->GetLinearVelocity().z });
	counter -= dt;
}

void AiStatemachineObject::Moveforward(float dt) {
	GetPhysicsObject()->AddForce({ 0 , 0 , 10 });
	GetPhysicsObject()->SetLinearVelocity({ 0,0, GetPhysicsObject()->GetLinearVelocity().z });
	counter -= dt;
}

void AiStatemachineObject::getPositionfromobject(Vector3 objectpostion)
{
	obstaclespositions = objectpostion;
}

//void AiStatemachineObject::TestPathfinding() {
//	NavigationGrid grid("testGrid.txt");
//
//	NavigationPath outPath;
//
//	
//
//	//Vector3 startPos(360, 8, 380);
//	Vector3 startPos = testStateObject->GetTransform().GetPosition();
//	//Vector3 endPos(360, 8, 0);
//	Vector3 endPos = Vector3(180, 0, 220);
//	bool found = grid.FindPath(startPos, endPos, outPath);
//
//	Vector3 pos;
//	while (outPath.PopWaypoint(pos)) {
//		testNodes.push_back(pos);
//	}
//	outPath.Clear();
//
//}
//
//void AiStatemachineObject::DisplayPathfinding() {
//	for (int i = 1; i < testNodes.size(); ++i) {
//		Vector3 a = testNodes[i - 1];
//		Vector3 b = testNodes[i];
//		Debug::DrawLine(a - Vector3(200, 0, 200), b - Vector3(200, 0, 200), Debug::RED);
//	}
//	if (testNodes.size() > 1) {
//		testStateObject->GetPhysicsObject()->AddForce((testNodes[1] - Vector3(200, 0, 200) - testStateObject->GetTransform().GetPosition()) * 2);
//		testNodes.clear();
//	}
//}


void AiStatemachineObject::ObjectDetectRay(GameObject* gameObject) {

	Vector3 objectPosition = gameObject->GetTransform().GetPosition();
	Vector3 objectForward = gameObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Ray ray(objectPosition, objectForward);

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

	RayCollision closestCollision;
	//	closestCollision.rayDistance = 100.0f;

	float shortDistance = 999999;
	for (auto ray : rays) {
		if (world->Raycast(ray, closestCollision, true, gameObject)) {
			GameObject* ObjectHited = (GameObject*)closestCollision.node;

			if (ObjectHited)
			{
				if (ObjectHited->gettag() == "Projectile" && closestCollision.rayDistance < 10.0f)
				{
					//std::cout << "Object detected";
					float distance = (ObjectHited->GetTransform().GetPosition() - objectPosition).Length();

					if (distance < shortDistance) {
						projectileToChase = ObjectHited;
						shortDistance = distance;
					}
					//ObjectHited->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * 100000, closestCollision.collidedAt);	
				}
			}
		}
	}

	Chasethebullets(0.2);
}

void AiStatemachineObject::Chasethebullets(float dt) {

	if (projectileToChase == nullptr) return;

	Vector3 movementDirection = projectileToChase->GetTransform().GetPosition() - this->GetTransform().GetPosition();
	movementDirection.Normalised();

	this->GetPhysicsObject()->SetLinearVelocity(movementDirection * 10 * 0.2);
	counter -= dt;
}