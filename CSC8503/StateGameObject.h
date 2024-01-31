#pragma once
#include "GameObject.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class StateGameObject : public GameObject  {
        public:
            StateGameObject();
            ~StateGameObject();

            virtual void Update(float dt);

            virtual void OnCollisionBegin(GameObject* otherObject) override;

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);
            void Jump(float dt);
            void lookaround(float dt);

            StateMachine* stateMachine;


            ParallelBehaviour* whattodo =
                new ParallelBehaviour("iamlost");

            float lookTimer;
            float jumpTimer;
            float leftTimer;
            float rightTimer;
            float waitTimer;
            float behaviourTimer;


            BehaviourState state = Ongoing;
            float counter;
        };
    }
}
