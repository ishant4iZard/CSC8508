#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Player.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {

	BehaviourAction* Lookaround = new BehaviourAction("Looking Around",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				//std::cout << "Looking for a key!\n";
				lookTimer = rand() % 3;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				lookTimer -= dt;
				if (lookTimer > 0.0f) {
					//std::cout << "look";
					lookaround(dt);
				}
				else
				{
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	BehaviourAction* wait = new BehaviourAction("wait",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				//std::cout << "Looking for a key!\n";
				waitTimer = 2000;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				waitTimer -= dt;
				if (waitTimer > 0.0f) {
					//std::cout << "wait";
				}
				else
				{
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);
	BehaviourAction* GoLeft = new BehaviourAction("Going Left",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				//std::cout << "Looking for 2 key!\n";

				leftTimer = rand() % 8;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				leftTimer -= dt;
				if (leftTimer > 0.0f) {
					//std::cout << "left";

					MoveLeft(dt);
				}
				else
				{
					return Success;
				}

			}
			return state; // will be ’ongoing ’ until success
		}
	);
	BehaviourAction* GoRight = new BehaviourAction("Going Right",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				//std::cout << "Looking for 3 key!\n";

				rightTimer = rand() % 8;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				rightTimer -= dt;
				if (rightTimer > 0.0f) {
					//std::cout << "right";

					MoveRight(dt);
				}
				else
				{
					return Success;
				}

			}
			return state; // will be ’ongoing ’ until success
		}
	);
	BehaviourAction* Jumping = new BehaviourAction("Jumping",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				//std::cout << "Looking for 4 key!\n";

				jumpTimer = rand() % 4;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				jumpTimer -= dt;
				if (jumpTimer > 0.0f) {
					//std::cout << "jump";

					Jump(dt);
				}
				else
				{
					return Success;
				}

			}
			return state; // will be ’ongoing ’ until success
		}
	);
	BehaviourSequence* lefty =
		new BehaviourSequence("lefty");
	lefty->AddChild(GoLeft);
	lefty->AddChild(wait);
	lefty->AddChild(Jumping);
	lefty->AddChild(wait);


	BehaviourSequence* righty =
		new BehaviourSequence("righty");
	righty->AddChild(GoRight);
	righty->AddChild(wait);
	righty->AddChild(Jumping);
	righty->AddChild(wait);


	BehaviourSelector* move = new BehaviourSelector("move");

	move->AddChild(lefty);
	move->AddChild(righty);

	BehaviourSelector* Check = new BehaviourSelector("Check");
	Check->AddChild(Lookaround);
	Check->AddChild(wait);


	/*whattodo->AddChild(lefty);
	whattodo->AddChild(righty);*/

	whattodo->AddChild(Check);
	whattodo->AddChild(move);
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	if(state == Ongoing && behaviourTimer<0) {
		//behaviourTimer -= dt;
		state = whattodo->Execute(dt);
	}
	if(state == Failure||state == Success) {
		whattodo->Reset();
		behaviourTimer = rand() % 9;
		state = Ongoing;
	}
	else
	{
		behaviourTimer -=dt;
	}
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->SetLinearVelocity(Vector3(-2, 0, 0) );
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->SetLinearVelocity(Vector3(2, 0, 0) );
}

void StateGameObject::Jump(float dt) {
	GetPhysicsObject()->AddForce(Vector3(0, 50, 0) );

}

void StateGameObject::lookaround(float dt) {
	if(rand()%20 <10)
		GetPhysicsObject()->AddTorque(Vector3(0, 2, 0) );
	else
		GetPhysicsObject()->AddTorque(Vector3(0, -2, 0));
}

void StateGameObject::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->gettag() == "Player")
	{
		Player* player = dynamic_cast<Player*>(otherObject);
		player->TakeDamage(2);
	}
}
