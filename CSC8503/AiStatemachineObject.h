#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class AiStatemachineObject : public GameObject {
        public:
            AiStatemachineObject();
            ~AiStatemachineObject();
            void getPositionfromobject(Vector3 objectposition);
            virtual void Update(float dt);
            

        protected:
            void MoveRound(float dt);
            void MoveRight(float dt);
            void Moveforward(float dt);
            StateMachine* stateMachine;
            float counter;
            Vector3 obstaclespositions;
        };
    }
}
