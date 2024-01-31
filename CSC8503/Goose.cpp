#include "Goose.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "Player.h"
#include "PhysicsObject.h"

using namespace NCL::CSC8503;

Goose::Goose(GameWorld& inworld, Player* inplayer ,const std::string& objectName) : world(inworld){
	name = objectName;
	worldID = -1;
	isActive = true;
	boundingVolume = nullptr;
	physicsObject = nullptr;
	renderObject = nullptr;
	networkObject = nullptr;
	tag = "Enemy";
	player = inplayer;

	grid = new NavigationGrid("Maze2.txt");

	mazePos = Vector3(35.0f, -20.0f, 0.0f);

	stateMachine = new StateMachine();
	Idle = new State([&](float dt)->void
		{
			std::cout << "idle";
		}
	);

	Roaming = new State([&](float dt)->void
		{
			
			if (pathList.size() > 0)
			{
				//std::cout << "roaming";
				//DebugDisplayPath(pathList);
				float nodeDistance = (pathList[currentDestinationIndex] - (transform.GetPosition())).Length();
				if (nodeDistance <= distanceThreshold)
				{
					//std::cout << "destination......." << currentDestinationIndex;
					currentDestinationIndex++;
					if (currentDestinationIndex >= (int)pathList.size() - 1)
						FindRandomPatrolPoint();
				}

				MoveTowards(pathList[currentDestinationIndex], dt, true);
				RotateTowards(pathList[currentDestinationIndex], rotationSpeed, dt);
			}
		}
	);

	Chasing = new State([&](float dt)->void
		{
			if (player == nullptr)
				return;



			currentDestination = player->GetTransform().GetPosition();
			if (LookingAtPlayer())
			{
				MoveTowards(currentDestination, dt, true);
				RotateTowards(currentDestination, rotationSpeed, dt);

				pathList.clear();
			}
			else
			{
				pathTimerCurrent += dt;
				if (pathTimerCurrent >= pathTimer)
				{
					FindPath(currentDestination, pathList);
					currentDestinationIndex = 0;
					pathTimerCurrent = 0.0f;
				}
			}

			//DebugDisplayPath(pathList);

			if (pathList.size() > 0)
			{
				float nodeDistance = (pathList[currentDestinationIndex] - (transform.GetPosition() + basePos)).Length();
				if (nodeDistance <= distanceThreshold)
					currentDestinationIndex = (currentDestinationIndex + 1) % pathList.size();

				MoveTowards(pathList[currentDestinationIndex], dt, false);
				RotateTowards(pathList[currentDestinationIndex], rotationSpeed, dt);
			}
			
		}
	);
	StateTransition* stateIdleToRoaming = new StateTransition(Idle, Roaming, [&](void)->bool
		{
			FindRandomPatrolPoint();
			return true;
		}
	);

	StateTransition* stateRoamingToChasing = new StateTransition(Roaming, Chasing, [&](void)->bool
		{
			float distance = (mazePos - player->GetTransform().GetPosition()).LengthSquared();
			return distance < (mazeRadius * mazeRadius);
		}
	);

	StateTransition* stateChasingToRoaming = new StateTransition(Chasing, Roaming, [&](void)->bool
		{
			float distance = (mazePos - player->GetTransform().GetPosition()).LengthSquared();
			if (distance > (mazeRadius * mazeRadius))
			{
				FindRandomPatrolPoint();
				return true;
			}
			else
				return false;
		}
	);

	stateMachine->AddState(Idle);
	stateMachine->AddState(Roaming);
	stateMachine->AddState(Chasing);
	stateMachine->AddTransition(stateIdleToRoaming);
	stateMachine->AddTransition(stateRoamingToChasing);
	stateMachine->AddTransition(stateChasingToRoaming);
}

void Goose::Update(float dt)
{
	//std::cout << transform.GetPosition().y << std::endl;
	stateMachine->Update(dt);
}

void Goose::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->gettag() == "Player")
	{
		player->TakeDamage(meleeDamage);
		if (player->GetHealth() <= 0.0f)
			FindRandomPatrolPoint();		//Go Back to Patrol State
	}
}

void Goose::DebugDisplayPath(std::vector<Vector3> paths)
{
	for (int i = 1; i < paths.size(); ++i)
	{
		Vector3 a = paths[i - 1];
		Vector3 b = paths[i];

		Debug::DrawLine(a, b, Debug::GREEN);
	}
}

bool Goose::LookingAtPlayer()
{
	bool status = false;

	Vector3 playerDir = currentDestination - transform.GetPosition();
	Ray ray = Ray(transform.GetPosition(), playerDir.Normalised());
	RayCollision rayCol;

	if (world.Raycast(ray, rayCol, true, this))
		status = rayCol.node == player;

	return status;
}

void Goose::FindRandomPatrolPoint()
{
	std::cout << "random";
	currentDestination = GetRandomMazePoint();
	FindPath(currentDestination, pathList);
	currentDestinationIndex = 0;
}

Vector3 Goose::GetRandomMazePoint() {
	int i = rand() % 5;
	switch (i)
	{
	case 1:
		return Vector3(40, -18.8, -20);
	case 2:
		return Vector3(26, -18.8, 20);
	case 3:
		return Vector3(40, -18.8, 20);
	case 4:
		return Vector3(10, -18.8, 20);
	case 0:
		return Vector3(60, -18.8, -20);

	default:
		break;
	}
}

void Goose::MoveTowards(Vector3 src, const Vector3& pos, float dt)
{
	
	Vector3 v = (pos - src).Normalised();
	v.y = 0.0f;
	physicsObject->AddForce(v * moveSpeed);

	previousPosition = transform.GetPosition();
}

void Goose::MoveTowards(const Vector3& pos, float dt, bool useForce)
{
	if (useForce)
	{
		Vector3 v = (pos - transform.GetPosition()).Normalised();
		v.y = 0.0f;
		physicsObject->AddForce(v * moveSpeed);
	}
	else
		transform.SetPosition(Vector3::MoveTowards(transform.GetPosition(), pos, moveSpeed * dt));
}

void Goose::RotateTowards(const Vector3& pos, float rotSpeed, float dt)
{
	Matrix4 rotMat = Matrix4::BuildViewMatrix(transform.GetPosition(), pos, Vector3(0, 1, 0)).Inverse();
	Quaternion ogRot = Quaternion(rotMat);
	Vector3 ogRotEuler = ogRot.ToEuler();
	ogRotEuler.x = 0;
	ogRotEuler.z = 0;
	Quaternion finalRot = Quaternion::EulerAnglesToQuaternion(ogRotEuler.x, ogRotEuler.y, ogRotEuler.z);
	transform.SetOrientation(Quaternion::Slerp(transform.GetOrientation(), finalRot, rotSpeed * dt));
}

void Goose::RotateAway(const Vector3& pos, float rotSpeed, float dt)
{
	Quaternion ogRot = Quaternion::RotateTowards(transform.GetPosition(), pos, Vector3(0, 1, 0));
	Vector3 ogRotEuler = ogRot.ToEuler();
	ogRotEuler.x = 0;
	ogRotEuler.y -= 180.0f;
	ogRotEuler.z = 0;
	Quaternion finalRot = Quaternion::EulerAnglesToQuaternion(ogRotEuler.x, ogRotEuler.y, ogRotEuler.z);
	transform.SetOrientation(Quaternion::Slerp(transform.GetOrientation(), finalRot, rotSpeed * dt));
}

bool Goose::FindPath(const Vector3& destination, std::vector<Vector3>& _pathList)
{
	Vector3 offset = Vector3(5, 0, -31);
	path.Clear();
	if (grid->FindPath((transform.GetPosition() - offset), (destination - offset), path))
	//if (grid->FindPath(Vector3(56, 20, 58),Vector3(0,208,3), path))
	{
		
		if (_pathList.size() > 0) _pathList.clear();
		int i = 0;
		Vector3 pos;
		while (path.PopWaypoint(pos)) {
			_pathList.push_back(pos);
			i++;
		}
		//std::cout << "path"<<i<<"\n";
		return true;
	}
	return false;
}