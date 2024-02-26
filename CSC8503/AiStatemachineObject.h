#pragma once
#include "GameObject.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "PhysicsSystem.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class AiStatemachineObject : public GameObject {
        public:
            AiStatemachineObject(GameWorld* world, NavigationGrid* navGrid);
            ~AiStatemachineObject();

            void OnCollisionBegin(GameObject* otherObject) override;

            virtual void Update(float dt);

        protected:
            StateMachine* stateMachine;
            NavigationGrid* navGrid;
            const float DETECTION_RADIUS = 30.0f;
            const float DEACTIVATION_DISTANCE = 1.0f;
            GameWorld* world;
            float distanceToNearestProj = INT_MAX;
            GameObject* projectileToChase;

            void DetectProjectiles(GameObject* gameObject,float dt);
            void MoveRandomly(float dt);
            void ChaseClosestProjectile(float dt);
        };
    }
}
