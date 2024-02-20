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
            AiStatemachineObject(GameWorld* world);
            ~AiStatemachineObject();
            void getPositionfromobject(Vector3 objectposition);

            void ObjectDetectRay(GameObject* gameObject);
            void Chasethebullets(float dt);

            void TestPathfinding();
            void DisplayPathfinding();

            virtual void Update(float dt);

            

        protected:
            void MoveRound(float dt);
            void MoveRight(float dt);
            void Moveforward(float dt);
            StateMachine* stateMachine;
            float counter;
            Vector3 obstaclespositions;
            vector < Vector3 > testNodes;
            GameWorld* world;

            GameObject* projectileToChase;

           // AiStatemachineObject* testStateObject = nullptr;
        };
    }
}
