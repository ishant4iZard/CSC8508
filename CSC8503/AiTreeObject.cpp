#include "AiTreeObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "string.h"

using namespace NCL;
using namespace CSC8503;

AiTreeObject::AiTreeObject(std::string name) {

	BehaviourAction* Patrol = new BehaviourAction("Patrol",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				TestTimer = rand() % 4;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				TestTimer -= dt;
				if (TestTimer > 0.0f) {
					PatrolAction(dt);
				}
				else
				{
					return Success;
				}
			}
			return state;
		}
	);

	BehaviourAction* Closetobullet = new BehaviourAction("Closetobullet",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				//std::cout << "Looking for 4 key!\n";
				TestTimer = rand() % 4;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				TestTimer -= dt;
				if (TestTimer > 0.0f) {
					//std::cout << "jump";
					ClosetobulletAction(dt);
				}
				else
				{
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	BehaviourAction* Reboundbullet = new BehaviourAction("Reboundbullet",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				//std::cout << "Looking for 4 key!\n";

				TestTimer = rand() % 4;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				TestTimer -= dt;
				if (TestTimer > 0.0f) {
					//std::cout << "jump";

					ReboundbulletAction(dt);
				}
				else
				{
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	BehaviourAction* Chasecloserone = new BehaviourAction("Chasecloserone",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				TestTimer = rand() % 4;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				TestTimer -= dt;
				if (TestTimer > 0.0f) {
					ChasecloseroneAction(dt);
				}
				else
				{
					return Success;
				}
			}
			return state;
		}
	);

	BehaviourAction* Trytoattack = new BehaviourAction("Trytoattack",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				TestTimer = rand() % 4;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				TestTimer -= dt;
				if (TestTimer > 0.0f) {
					TrytoattackAction(dt);
				}
				else
				{
					return Success;
				}
			}
			return state;
		}
	);


	BehaviourSequence* CloseAndRebound =
		new BehaviourSequence("CloseAndRebound");
	CloseAndRebound->AddChild(Closetobullet);
	CloseAndRebound->AddChild(Reboundbullet);

	BehaviourSequence* CloserCloseAndRebound =
		new BehaviourSequence("CloserCloseAndRebound");
	CloserCloseAndRebound->AddChild(Chasecloserone);
	CloserCloseAndRebound->AddChild(CloseAndRebound);

	BehaviourSelector* SelectorLevel2 =
		new BehaviourSelector("SelectorLevel2");
	SelectorLevel2->AddChild(CloserCloseAndRebound);
	SelectorLevel2->AddChild(CloseAndRebound);

	BehaviourSequence* SequenceLevel2 =
		new BehaviourSequence("SequenceLevel2");
	SequenceLevel2->AddChild(Trytoattack);
	SequenceLevel2->AddChild(SelectorLevel2);

	BehaviourSelector* SelectorLevel1 =
		new BehaviourSelector("SelectorLevel1");
	SelectorLevel2->AddChild(SequenceLevel2);
	SelectorLevel2->AddChild(Patrol);


}

AiTreeObject::~AiTreeObject() {
	delete stateMachine;
}



void AiTreeObject::Update(float dt) {
	if(state == Ongoing && TestTimer<0) {
		BehaviorTestTimer -= dt;
		state = SelectorLevel1->Execute(dt);
	}
	if(state == Failure||state == Success) {
		SelectorLevel1->Reset();
		TestTimer = rand() % 9;
		state = Ongoing;
	}
	else
	{
		BehaviorTestTimer -=dt;
	}
}

void AiTreeObject::PatrolAction(float dt) {
	//cout << "patrol" << endl;
}

void AiTreeObject::ClosetobulletAction(float dt) {
	//cout << "Closetobullet" << endl;
}

void AiTreeObject::ReboundbulletAction(float dt) {
	//cout << "Reboundbullet" << endl;
}

void AiTreeObject::ChasecloseroneAction(float dt) {
	//cout << "ChasecloseroneAction" << endl;
}

void AiTreeObject::TrytoattackAction(float dt) {
	//cout << "TrytoattackAction" << endl;
}


void AiTreeObject::OnCollisionBegin(GameObject* otherObject)
{
	
}
