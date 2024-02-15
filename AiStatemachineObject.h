#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class StateGameObject : public GameObject {
        public:
            StateGameObject();
            ~StateGameObject();

            virtual void Update(float dt);

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);
            void Moveforward(float dt);
            StateMachine* stateMachine;
            float counter;


            const float ORBIT_RADIUS = 25.0; // Ensure that this value matches the distance of the player from the center
            const float ORBIT_SPEED = 100.0f;
            const Vector3 ORBIT_CENTER = Vector3(0, 0, 0);
        };
    }
}
