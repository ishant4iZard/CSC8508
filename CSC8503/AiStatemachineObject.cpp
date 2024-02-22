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

AiStatemachineObject::AiStatemachineObject(GameWorld* world) {
	counter = 0.0f;
	detectionOfBall = true;
	stateMachine = new StateMachine();

	this->world = world;

	State* PatrolState = new State([&](float dt) -> void
		{
			ObjectDetectRay(ObjectHited,dt);
			//AiDetectRay(this, dt);
			MoveRound(dt);
		}
	);
	State* ChaseState = new State([&](float dt) -> void
		{
			AiDetectRay(this, dt);
			Chasethebullets(dt);
		}
	);


	stateMachine->AddState(PatrolState);
	stateMachine->AddState(ChaseState);

	stateMachine->AddTransition(new StateTransition(PatrolState, ChaseState,
		[&]() -> bool
		{		
			return this->RaychangesDectec1();
		}
	));

	stateMachine->AddTransition(new StateTransition(ChaseState, PatrolState,
		[&]() -> bool
		{

		//return 	;
			if (counter <  0.0f) {

				//std::cout << "state two rayDistance: " << abc << "\n";
			}
			///GetPhysicsObject()->SetLinearVelocity({ 0,0,0 });
			return this->RaychangesDectec2();
			//return this->counter <  0.0f;
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
	//TestPathfinding();
	//DisplayPathfinding();
	//GetPhysicsObject()->AddForce({ 0 , 0 , -10 });
	//GetPhysicsObject()->SetLinearVelocity({ GetPhysicsObject()->GetLinearVelocity().x,0, GetPhysicsObject()->GetLinearVelocity().z });
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


void AiStatemachineObject::AiDetectRay(GameObject* gameObject,float dt) {
	//counter -= dt;
	//this->GetPhysicsObject()->SetLinearVelocity(movementDirection * 0 * 0.0);
	Vector3 objectPosition = gameObject->GetTransform().GetPosition();
	Vector3 objectForward = gameObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Ray ray(objectPosition, objectForward);

	const int numRays = 30;
	const float angleIncrement = 2 * 3.14 / numRays;

	vector<Ray> rays1;
	for (int i = 0; i < numRays; i++) {
		float angle = angleIncrement * i;
		float x = cos(angle);
		float z = sin(angle);

		Vector3 dir = Vector3(x, 0, z);
		rays1.push_back(Ray(objectPosition, dir));
		Debug::DrawLine(objectPosition, dir * 100, Debug::RED);
	}

	float shortDistance = 999999;
	bool projFound = false;
	 //abc = 999999;
	for (auto ray : rays1) {
		if (world->Raycast(ray, closestCollision, true, gameObject)) {
			GameObject* ObjectHited = (GameObject*)closestCollision.node;

			if (ObjectHited)
			{
				if (ObjectHited->gettag() == "Projectile" 
					//&& closestCollision.rayDistance < 20.0f
					)
				{
					//std::cout << "Object detected";
					float distance = (ObjectHited->GetTransform().GetPosition() - objectPosition).Length();
						//std::cout << "Dist " << abc << "\n";

					if (distance < shortDistance) {
						projectileToChase = ObjectHited;
						shortDistance = distance;
						abc = distance;
						projFound = true;
					}
					//ObjectHited->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * 100000, closestCollision.collidedAt);	
				}
			}
		}
	}
	if (!projFound) abc = 9999999;
}

void AiStatemachineObject::ObjectDetectRay(GameObject* floor,float dt) {
	//Vector3 objectPosition = Vector3(-98, 10, -98);
	//Vector3 objectForward = GetTransform().GetOrientation() * Vector3(0, -1, 0);
	 int x = -98;
	 const int numRays2 =50;
	vector<Ray> rays2;
	for (int i = 0; i < numRays2; i++)
	{
		x += 4;
		int z = -98;
		for (int j = 0; j < numRays2; j++)
		{
			z += 4;
			//std::cout <<x<< "youmeiyou" <<z<< "\n";
			Ray ray(Vector3(x, 20, z), Vector3(x, 0, z));
			rays2.push_back(Ray(Vector3(x, 20, z), Vector3(x, 0, z)));
			Debug::DrawLine(Vector3(x, 20, z), Vector3(x, 0, z) * 100, Debug::RED);
		}

	}

	for (auto ray : rays2) {
		if (world->Raycast(ray, obstaclesCollision, true, floor)) {
			GameObject* ObjectDetected = (GameObject*)closestCollision.node;

			if (ObjectDetected)
			{
				if (ObjectDetected->gettag() == "walls"&& ObjectDetected->gettag() == "bouncingpad1"
					//&& closestCollision.rayDistance < 20.0f
					)
				{
					std::cout << "Object detected";
					//ObjectHited->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * 100000, closestCollision.collidedAt);	
				}
			}
		}
	}
	//Debug::DrawLine(objectPosition, Vector3(-98, 0, -98), Debug::RED);

	//const int numRays1 = 30;
	//const float angleIncrement = 2 * 3.14 / numRays2;
}


bool AiStatemachineObject::RaychangesDectec1()
{
	//std::cout << "Chasea " << abc << "\n";
	if (abc < 30.0f 
		//&& k==1
		)
	{
		k = 0;
		//std::cout << "k: " << k << "\n";
		return true;
	}
	return false;
}

bool AiStatemachineObject::RaychangesDectec2()
{
	//std::cout << "Patrol " << abc << "\n";
	if (abc > 30.0f
		//&& k == 0
		)
	{
		k = 1;
		//std::cout << "k: " << k << "\n";
		return true;
	}
	return false;
}

void AiStatemachineObject::Chasethebullets(float dt) {
	const static int SPEED =2000;
	if (projectileToChase == nullptr) return;

    movementDirection = projectileToChase->GetTransform().GetPosition() - this->GetTransform().GetPosition();

	this->GetPhysicsObject()->SetLinearVelocity(movementDirection.Normalised() * SPEED * dt);

	this->OnCollisionBegin(projectileToChase);
}

void AiStatemachineObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->gettag() == "Projectile") {
		CollisionDetection::CollisionInfo info;
		CollisionDetection::ObjectIntersection(this, otherObject, info);
		Projectile* Bullet = dynamic_cast<Projectile*>(otherObject);
		//Bullet->GetOwner()->AddScore(1);
		if (abc < 3.0) {
			std::cout << "Eat " << abc << "\n";
		Bullet->deactivate();
		}

		DeactivateProjectilePacket newPacket;
		//newPacket.NetObjectID = Bullet->GetNetworkObject()->GetNetworkID();
		//if (Bullet->GetGame()->GetServer())
		//{
		//	Bullet->GetGame()->GetServer()->SendGlobalPacket(newPacket);
		//}
	}
}

