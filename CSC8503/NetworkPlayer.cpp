#include "NetworkPlayer.h"
#include "NetworkedGame.h"
#include "PhysicsObject.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"


using namespace NCL;
using namespace CSC8503;

//NetworkPlayer* NetworkPlayer::createAngryGoose(NetworkedGame* game, int num)
//{
//	NetworkPlayer* goose = new NetworkPlayer(game, num);
//
//	goose->stateMachine = new StateMachine();
//
//	State* patrol = new State(
//		[&](float dt)->void
//		{
//			Vector3 center = Vector3(4, 0, -4);
//			goose->AIMoveTo(center, dt);
//		}
//	);
//	 
//	goose->stateMachine->AddState(patrol);
//	return goose;
//}

Vector3 patrolPoints[4] = {
	Vector3(-108, 0,-108),
	Vector3( 108, 0,-108),
	Vector3( 108, 0, 108),
	Vector3(-108, 0, 108)
};
float waitT = 0.5f;


NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num)	{
	this->game = game;
	playerNum  = num;

	sprintTimer = SprintCDT;
	fireTimer = FireCDT;
	haveTreasure = false;
	score = 0;
}

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num, int AIKind)
{
	this->game = game;
	playerNum = num;
	sprintTimer = SprintCDT;
	fireTimer = FireCDT;
	haveTreasure = false;
	score = 0;
	if (AIKind == 1)
	{
		stateMachine = new StateMachine();

		State* patrol = new State(
			[&](float dt)->void
			{
				//Debug::DrawLine(patrolPoints[patrolIndex], patrolPoints[patrolIndex] + Vector3(0, 50, 0), Debug::GREEN);
				if (this->AIMoveTo(patrolPoints[patrolIndex], dt)) 
				{
					waitT -= dt;
					if (waitT < 0.0f)
					{
						++patrolIndex;
						patrolIndex = patrolIndex % 4;
						waitT = 0.5f;
						//std::cout << "patrolIndex = " << patrolIndex << std::endl;
					}
				}
				this->UpdateVisualList(dt);
			}
		);

		State* chasePlayer = new State(
			[&](float dt)->void
			{
				this->UpdateVisualList(dt);
				NetworkPlayer* target = this->getVisualTarget();
				if (target != nullptr)
				{
					Vector3 pos = target->GetTransform().GetPosition();
					AIMoveTo(pos, dt);
				}
			}
		);

		StateTransition* patrolToChase = new StateTransition(patrol, chasePlayer,
			[&](void)->bool
			{
				if (this->getVisualTarget() != nullptr)
				{
					return true;
				}
				else {
					return false;
				}
			}
		);

		StateTransition* chaseToPatrol = new StateTransition(chasePlayer, patrol,
			[&](void)->bool
			{
				if (this->getVisualTarget() == nullptr)
				{
					return true;
				}
				else {
					return false;
				}
			}
		);

		stateMachine->AddState(patrol);
		stateMachine->AddState(chasePlayer);
		stateMachine->AddTransition(patrolToChase);
		stateMachine->AddTransition(chaseToPatrol);
	}
	else if (AIKind == 2)
	{
		CreateUndercoverAgent();
	}
}

NetworkPlayer::~NetworkPlayer()	{
	delete stateMachine;
}

void NetworkPlayer::OnCollisionBegin(GameObject* otherObject) {
	if (game) {
		if (dynamic_cast<NetworkPlayer*>(otherObject))
		{
			game->OnPlayerCollision(this, (NetworkPlayer*)otherObject);
		}
	}
}

void NetworkPlayer::GameTick(float dt)
{
	UpdateTimer(dt);
}

void NetworkPlayer::ExcuteBehavioursTree(float dt)
{
	if(rootSequence != nullptr)
	{
		BehaviourState state = rootSequence->Execute(dt);
		switch (state)
		{
		case Success:
			rootSequence->Reset();
			break;
		case Failure:
			rootSequence->Reset();
			break;
		}
	}
}

Quaternion GenerateOrientation(const Vector3& axis, float angle)
{
	float halfAngle = angle / 2;
	float s = std::sin(halfAngle);
	float w = std::cos(halfAngle);
	float x = axis.x * s;
	float y = axis.y * s;
	float z = axis.z * s;
	return Quaternion(x, y, z, w);
}

void NetworkPlayer::SetPlayerYaw(const Vector3& pointPos)
{
	Quaternion orientation;
	Vector3 pos = transform.GetPosition();
	Vector3 targetForwardVec = (pointPos - pos);
	targetForwardVec.y = 0;
	targetForwardVec = targetForwardVec.Normalised();

	Vector3 forward = Vector3(0, 0, -1);

	float cosTheta = Vector3::Dot(forward, targetForwardVec);
	Vector3 rotationAxis;
	float angle;
	if (cosTheta < -1 + 0.001f) 
	{
		rotationAxis = Vector3::Cross(Vector3(0, 0, 1), forward);
		if (rotationAxis.Length() < 0.01)
		{
			rotationAxis = Vector3::Cross(Vector3(1, 0, 0), forward);
		}
		rotationAxis = rotationAxis.Normalised();
		angle = 3.1415926f;
	}
	else
	{
		rotationAxis = Vector3::Cross(forward, targetForwardVec);
		rotationAxis = rotationAxis.Normalised();
		angle = std::acos(cosTheta);
	}
	orientation = GenerateOrientation(rotationAxis, angle);

	transform.SetOrientation(orientation);

	//Debug::DrawLine(pos, pos + forward * 30, Debug::MAGENTA);
}

void NetworkPlayer::MovePlayer(bool Up, bool Down, bool Right, bool Left)
{
	Vector3 MoveDir = Vector3(0, 0, 0);
	if (Up)    { MoveDir += Vector3( 0, 0, -1); }
	if (Down)  { MoveDir += Vector3( 0, 0,  1); }
	if (Right) { MoveDir += Vector3( 1, 0,  0); }
	if (Left)  { MoveDir += Vector3(-1, 0,  0); }
	MoveDir = MoveDir.Normalised();

	/*Vector3 currentVel = physicsObject->GetLinearVelocity();
	float currentSpeed = currentVel.Length();
	float k = 20;
	Vector3 resistence = currentVel.Normalised() * (currentSpeed * currentSpeed) * (-k);
	this->physicsObject->AddForce(resistence);*/
	physicsObject->setLinearDamp(2.0f);
	float maxSpeed = 10.0f;
	float f = 2000.0f;
	if (physicsObject->GetLinearVelocity().Length() > maxSpeed)
	{
		f = 0.0f;
	}
	Vector3 force = MoveDir * f;

	//std::cout << "velocity : " << physicsObject->GetLinearVelocity().Length() << std::endl;
	this->physicsObject->AddForce(force);
	//Debug::DrawLine(this->transform.GetPosition(), this->transform.GetPosition() + force, Debug::RED, 0.0f);
}

bool NetworkPlayer::AIMoveTo(Vector3 destination, float dt)
{
	Vector3 currentPos = transform.GetPosition();
	float distance = (destination - currentPos).Length();
	if (distance < 3.0f)
	{
		return true;// have arrived the destination;
	}

	pathfindingTimer -= dt;
	if (pathfindingTimer <= 0.0f)
	{
		waypoints.clear();
		if (game->findPathToDestination(currentPos, destination, waypoints) == false)
		{
			return true;
		}
		waypoint = waypoints.begin() + 1;
		pathfindingTimer = 3.0f;
	}
	if (waypoint == waypoints.end()) 
	{ 
		pathfindingTimer = 0.0f;
		return true; 
	}
	SetPlayerYaw(*waypoint);
	if (AIMove(*waypoint)) { ++waypoint; }

	return false;
}

bool NetworkPlayer::AIMove(Vector3 destination)
{
	Vector3 MoveDir = destination - transform.GetPosition();
	if (MoveDir.Length() < 2.0f)
	{
		return true;// have arrived the destination;
	}
	MoveDir.Normalise();
	bool move[4] = { 0,0,0,0 };
	if (MoveDir.z < 0) { move[Up] = 1; }
	if (MoveDir.z > 0) { move[Down] = 1; }
	if (MoveDir.x > 0) { move[Right] = 1; }
	if (MoveDir.x < 0) { move[Left] = 1; }
	MovePlayer(move[Up], move[Down], move[Right], move[Left]);

	return false; //have not arrived the destination
}

NetworkPlayer* NetworkPlayer::AIvision()
{
	Vector3 currentPos = transform.GetPosition();
	Ray sightLine(currentPos, this->getPlayerForwardVector());

	//Debug::DrawLine(currentPos, currentPos + getPlayerForwardVector() * 50, Debug::CYAN);

	RayCollision closestCollision;
	GameWorld* world = game->getGameWorld();
	if (world->Raycast(sightLine, closestCollision, true))
	{
		GameObject* hitObject = (GameObject*)closestCollision.node;
		if (dynamic_cast<NetworkPlayer*>(hitObject))
		{
			return (NetworkPlayer*)hitObject;
		}
	}
	return nullptr;
}

void NetworkPlayer::UpdateVisualList(float dt)
{
	NetworkPlayer* sawPlayer = AIvision();
	bool flag = true;
	if (sawPlayer != nullptr)
	{
		if (sawPlayer->GetPlayerNum() > 3)
		{
			sawPlayer = nullptr; // AI not count
		}
		else
		{
			flag = false;
		}
	}

	for (auto i = viualList.begin(); i != viualList.end(); )
	{
		if (sawPlayer != nullptr) // not an AI
		{
			if (sawPlayer == i->first)
			{
				i->second = 3.0f;
				flag = true;
			}
		}
		i->second -= dt;
		if (i->second < 0)
		{
			i = viualList.erase(i);
		}
		else {
			++i;
		}
	}

	if (!flag)
	{
		viualList.push_back(std::pair<NetworkPlayer*, float>(sawPlayer, 3.0 - dt));
	}
}

NetworkPlayer* NetworkPlayer::getVisualTarget()
{
	auto itr = viualList.begin();
	if (itr == viualList.end()) {
		return nullptr;
	}
	return itr->first;
}

void NetworkPlayer::PlayerSprint()
{
	if (sprintTimer <= 0.0f)
	{
		Vector3 sprintDir = getPlayerForwardVector();
		float f = 500000.0f;
		Vector3 force = sprintDir * f;
		this->physicsObject->AddForce(force);
		//Debug::DrawLine(transform.GetPosition(), transform.GetPosition() + force, Debug::GREEN, 3.0f);
		sprintTimer = SprintCDT;
	}
}

void NetworkPlayer::PlayerFire()
{
	if (fireTimer <= 0)
	{
		Vector3 fireDir = getPlayerForwardVector();
		Vector3 firePos = transform.GetPosition() + fireDir * 10 + Vector3(0, 2, 0);
		game->SpawnBullet(this, firePos, fireDir);
		//Debug::DrawLine(transform.GetPosition(), transform.GetPosition() + fireDir * 20.0f, Debug::CYAN, 3.0f);
		fireTimer = FireCDT;
	}
}

Vector3 NetworkPlayer::getPlayerForwardVector()
{
	Vector3 vec = Vector3(0, 0, -1);
	vec = transform.GetOrientation() * vec;
	vec = vec.Normalised();
	return vec;
}

void NetworkPlayer::UpdateTimer(float dt)
{
	sprintTimer -= dt;
	fireTimer -= dt;

	if (sprintTimer < 0) { sprintTimer = 0; }
	if (fireTimer < 0) { fireTimer = 0; }
}

void NetworkPlayer::CreateUndercoverAgent()
{
	BehaviourAction* patrol = new BehaviourAction("Patrol",
		[&](float dt, BehaviourState state)->BehaviourState
		{
			if (state == Initialise)
			{
				patrolIndex = 0;
				state = Ongoing;
			}
			else if (state == Ongoing)
			{
				if (this->AIMoveTo(patrolPoints[patrolIndex], dt))
				{
					patrolIndex++;
					patrolIndex = patrolIndex % 4;
				}
				if (targetPlayer != nullptr)
				{
					return Success;
				}
			}
			return state;
		}
	);

	BehaviourAction* searchPlayer = new BehaviourAction("Search Player",
		[&](float dt, BehaviourState state)->BehaviourState
		{
			if (state == Initialise)
			{
				state = Ongoing;
			}
			else if (state == Ongoing)
			{
				this->UpdateVisualList(dt);
				if (this->getVisualTarget() != nullptr)
				{
					if (this->getVisualTarget()->GetPlayerNum() < 4)
					{
						targetPlayer = this->getVisualTarget();
						return Success;
					}
				}
			}
			return state;
		}
	);

	BehaviourAction* chaseTarget = new BehaviourAction("Chase Target",
		[&](float dt, BehaviourState state)->BehaviourState
		{
			if (state == Initialise)
			{
				state = Ongoing;
			}
			else if (state == Ongoing)
			{
				Vector3 pos = targetPlayer->GetTransform().GetPosition();
				AIMoveTo(pos, dt);
			}
			return state;
		}
	);

	BehaviourParallel* parallel = new BehaviourParallel("Patrol Routine");
	parallel->AddChild(patrol);
	parallel->AddChild(searchPlayer);

	BehaviourSelector* selector = new BehaviourSelector("Chase Player");
	selector->AddChild(chaseTarget);

	rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(parallel);
	rootSequence->AddChild(selector);
}

