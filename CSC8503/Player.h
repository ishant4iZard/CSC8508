#pragma once
#include "GameObject.h"
#include <string>
#include "Vector3.h"
#include "GameWorld.h"

namespace NCL
{
    namespace CSC8503
    {
        class Player : public GameObject
        {
        public:
            Player(GameWorld& game, const std::string& objectName = "");

            virtual void Update(float dt);

            bool getDestroyVoxels() const {
                return destroyVoxels;
            }
            void setDestroyVoxels(bool destroy) {
                destroyVoxels = destroy;
            }

            void GiveGrapplePowerup(const float& powerupCharge)
            {
                ropePowerupTimer = powerupCharge;
                ropePowerupCurrent = ropePowerupTimer;
            }

            float GetScore() const { return score; }
            void AddScore(const float& scored) { score += scored; }

            float GetHealth() const { return currentHealth; }
            void TakeDamage(float damage)
            {
                currentHealth -= damage;
                if (currentHealth <= 0.0f)
                {
                    currentHealth = 0.0f;
                    ResetPlayer();
                }
            }
            bool grounded() {
                return isOnGround;
            }
            bool Jump() {
                return canJump;
            }

            void ResetPlayer(bool includeKey = false) {};

            bool hasDoorKey = false;

        protected:
            GameWorld& world;

            float currentHealth = 0.0f;
            float maxHealth = 100.0f;

            float moveSpeed = 15.0f;
            float rotationSpeed = 5.0f;
            float mass = 7.5f;
            float jumpForce = 7.0f;
            float airControl = 0.5f;

            bool isOnGround = true;

            bool isHooked = false;
            float ropePowerupTimer = 30.0f;
            float ropePowerupCurrent = 0.0f;
            bool ropePowerup = true;
            Vector3 ropeAnchorPoint;
            float ropeForce;
            float ropeDamping;

            float score = 0.0f;
            bool destroyVoxels = true;

            bool canJump = false;
        };

    }
}