#pragma once
#include "GameObject.h"
#include "GameWorld.h"
#include <vector>

enum AIState {
    CHASE,
    PATROL
};

namespace NCL {
    namespace CSC8503 {
        class NavigationGrid;
        class NavigationMesh;
        class PhysicsSystem;
        class Projectile;
        class StateMachine;

        class AiStatemachineObject : public GameObject {
        public:
            AiStatemachineObject(GameWorld* world, NavigationGrid* navGrid);
            ~AiStatemachineObject();

            void OnCollisionBegin(GameObject* otherObject) override;
            void OnCollisionEnd(GameObject* otherObject) override;

            virtual void Update(float dt);
            void DetectProjectiles(std::vector<Projectile*> ProjectileList);

        protected:
            StateMachine* stateMachine;
            NavigationGrid* navGrid;
            const float DETECTION_RADIUS = 50.0f;
            const int SPEED = 35;
            const float DEACTIVATION_DISTANCE = 1.0f;
            Vector3 randomMovementDirection;
            GameWorld* world;
            float distanceToNearestProj = INT_MAX;
            Projectile* projectileToChase;
            std::vector<Vector3> pathFromAIToPlayer;
            AIState currentState = PATROL;
            bool isCollidingWithProj;

            //void DetectProjectiles(GameObject* gameObject,float dt);
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
