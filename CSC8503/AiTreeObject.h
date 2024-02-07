#pragma once
#include "GameObject.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class AiTreeObject : public GameObject  {
        public:
            AiTreeObject(std::string name);
            ~AiTreeObject();

            virtual void Update(float dt);
            virtual void OnCollisionBegin(GameObject* otherObject) override;

        protected:

            void PatrolAction(float dt);
            void ClosetobulletAction(float dt);
            void ReboundbulletAction(float dt);
            void ChasecloseroneAction(float dt);
            void TrytoattackAction(float dt);


            BehaviourSelector* SelectorLevel1;
    
            StateMachine* stateMachine;

            float TestTimer;
            float BehaviorTestTimer;

            BehaviourState state = Ongoing;
            float counter;
        };
    }
}
