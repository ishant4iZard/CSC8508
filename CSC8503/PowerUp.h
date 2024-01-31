#pragma once
#include "GameObject.h"
#include <string>

namespace NCL
{
    namespace CSC8503
    {
        enum PowerUpState
        {
            Active,
            Cooldown,
            Disabled 
        };
        

        class Powerup : public GameObject
        {
        public:
            Powerup() {}
            Powerup(const std::string& name = "");

            virtual void Update(float dt);
            virtual void OnTriggerBegin(GameObject* otherObject) override;
            virtual void ResetPowerup();

            void setBonus(int a) {
                bonus = a;
            }

        protected:
            float elaspedTime = 0.0f;
            float yRot = 0.0f;
            float rotationSpeed = 50.0f;

            int bonus;
            float respawnTime = 35.0f;
            float respawnTimeCurrent = 45.0f;
            PowerUpState currentState = PowerUpState::Active;
            Vector3 scale;
        };

    }
}