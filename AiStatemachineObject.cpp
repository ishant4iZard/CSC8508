#include "AiStatemachineObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt) -> void
		{
			this->MoveLeft(dt);
		}
	);
	State* stateB = new State([&](float dt) -> void
		{
			this->MoveRight(dt);
		}
	);
	State* stateC = new State([&](float dt) -> void
		{
			this->Moveforward(dt);
		}
	);

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);
	stateMachine->AddState(stateC);

	stateMachine->AddTransition(new StateTransition(stateA, stateC,
		[&]() -> bool
		{
			return this->counter > 100.0f;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateC, stateB,
		[&]() -> bool
		{
			return this->counter > 1006.0f;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateB, stateA,
		[&]() -> bool
		{
			return this->counter < 0.0f;
		}
	));
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {

	Vector3 velocityDir = Vector3::Cross(Vector3(0, 1, 0), ORBIT_CENTER - this->GetTransform().GetPosition()).Normalised();
	this->GetPhysicsObject()->SetLinearVelocity(velocityDir * ORBIT_SPEED);
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 0 , 0 , 0 });
	counter -= dt;
}

void StateGameObject::Moveforward(float dt) {
	GetPhysicsObject()->AddForce({ 0 , 0 , 0 });
	counter += dt;
}