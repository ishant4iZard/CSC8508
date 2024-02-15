#include "AiStatemachineObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

AiStatemachineObject::AiStatemachineObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt) -> void
		{
			this->MoveRound(dt);
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
			return this->counter > 13.0f;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateC, stateB,
		[&]() -> bool
		{
			///GetPhysicsObject()->SetLinearVelocity({ 0,0,0 });
			return this->counter > 18.0f;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateB, stateA,
		[&]() -> bool
		{
		//GetPhysicsObject()->SetLinearVelocity({ 0,0,0 });
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

	GetPhysicsObject()->AddForce({ 10 , 0 , 0 });
	GetPhysicsObject()->SetLinearVelocity({ GetPhysicsObject()->GetLinearVelocity().x,0, 0 });
	/*Vector3 velocityDir = Vector3::Cross(Vector3(0, 1, 0), ORBIT_CENTER - this->GetTransform().GetPosition()).Normalised();
	this->GetPhysicsObject()->SetLinearVelocity(velocityDir * ORBIT_SPEED);*/
	counter += dt;
}

void AiStatemachineObject::MoveRight(float dt) {

	GetPhysicsObject()->AddForce({ 0 , 0 , -10 });
	GetPhysicsObject()->SetLinearVelocity({ 0,0, GetPhysicsObject()->GetLinearVelocity().z });

	counter -= dt;
}

void AiStatemachineObject::Moveforward(float dt) {

	GetPhysicsObject()->AddForce({ 0 , 0 , 10 });

	counter += dt;
}

void AiStatemachineObject::getPositionfromobject(Vector3 objectpostion)
{
	obstaclespositions = objectpostion;
}

