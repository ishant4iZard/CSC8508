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

            void AiDetectRay(GameObject* gameObject,float dt);
            void ObjectDetectRay(GameObject* floor, float dt);
            void Chasethebullets(float dt);

            bool RaychangesDectec1();
            bool RaychangesDectec2();


            void OnCollisionBegin(GameObject* otherObject);
           // void TestPathfinding();
          //  void DisplayPathfinding();

            virtual void Update(float dt);
            bool shouldDetectRayChanges = true;

            RayCollision  closestCollision;
            RayCollision  obstaclesCollision;
            vector<Ray> rays;
            GameObject* ObjectHited;

        protected:
            void MoveRound(float dt);
            void MoveRight(float dt);
            void Moveforward(float dt);
            StateMachine* stateMachine;
            
            bool detectionOfBall;
            float counter;
            Vector3 obstaclespositions;
            vector < Vector3 > testNodes;
            GameWorld* world;

            Vector3 movementDirection;
            float abc = 999999;
            bool k=1;
            GameObject* projectileToChase;



           // AiStatemachineObject* testStateObject = nullptr;
        };
    }
}
