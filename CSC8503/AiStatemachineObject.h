#pragma once
#include "GameObject.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "PhysicsSystem.h"
#include <vector>

enum AIState {
    CHASE,
    PATROL
};

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
            const float DETECTION_RADIUS = 50.0f;
            const int SPEED = 35;
            const float DEACTIVATION_DISTANCE = 1.0f;
            Vector3 randomMovementDirection;
            GameWorld* world;
            float distanceToNearestProj = INT_MAX;
            GameObject* projectileToChase;
            std::vector<Vector3> pathFromAIToPlayer;
            AIState currentState = PATROL;

            void DetectProjectiles(GameObject* gameObject,float dt);
            void MoveRandomly(float dt);
            void ChaseClosestProjectile(float dt);
            void FindPathFromAIToProjectile(float dt);
            void DisplayPathfinding();
            bool CanSeeProjectile();
            float timer = 0;
            const int TIME_TO_NEXT_UPDATE = 10;
        };
    }
}
