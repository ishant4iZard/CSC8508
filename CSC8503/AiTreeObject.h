#pragma once
#include "GameObject.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "Transform.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class AiTreeObject : public GameObject  {
        public:
            AiTreeObject(const std::string name);
            ~AiTreeObject();


            void BehaviorTree();
            void ResetBehaviourTree();
            virtual void Update(float dt);
            void OnBehaviour();

            virtual void OnCollisionBegin(GameObject* otherObject) override;




        protected:
            void PatrolAction(float dt, Vector3& targetPosition, float speed);
            void ClosetobulletAction(float dt);
            void ReboundbulletAction(float dt);
            void ChasecloseroneAction(float dt);
            void TrytoattackAction(float dt);
            
            void AIpathfindingtest(vector<Vector3>& waypoints);


            BehaviourSelector* SelectorLevel1;

           //StateMachine* stateMachine;

            float TestTimer;
            float BehaviorTestTimer;
            Vector3 testposition;

            BehaviourState state = Ongoing;
            float counter;
        };
    }
}
