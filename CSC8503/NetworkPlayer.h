#pragma once
#include "GameObject.h"
#include "GameClient.h"
#include <iostream>
#include "MaleGuard.h"
namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

		class NetworkPlayer : public GameObject {
		public:
			NetworkPlayer(NetworkedGame* game, int num);
			~NetworkPlayer();

			void OnCollisionBegin(GameObject* otherObject) override;

			void SetPlayerYaw(const Vector3& pointPos);

			void OscillatePlayer(float dt);
			void RotatePlayer(float dt);
			void MovePlayerInSquarePattern(float dt); // The player will move along the boundaries of the level in a square pattern
			void MovePlayerTowardsCursor(float dt);
			void ReplenishProjectiles(float dt);

			bool isFire = false;
			void Fire();

			void AddScore(int score) {
				Score += score;
				//std::cout << "Score: = " << Score;
			}

			int GetScore() {
				return Score;
			}

			void SetMovementDir(Vector3 dir) {
				movementDirection = dir;
			}

			void SetAnimation(int inAnimationID) { activeAnimation = maleGuardAnimations[inAnimationID]; }
			MeshAnimation* GetAnimation() { return activeAnimation; }

			MeshAnimation* GetDefalutAnimation() { return defaultAnimation; }

			void SetAnimationStateCounter() { animationStateCounter++; }
			int GetAnimationStateCounter() { return animationStateCounter; }

		protected:
			NetworkedGame* game;
			int playerNum;
			int Score = 0;
			float timeElapsed;
			float movementSpeed;
			Vector3 movementDirection;
			Vector3 pointPos;

			const int MAX_PROJECTILE_CAPACITY = 10;
			int numProjectilesAccumulated;
			float projectileReplenishTimer;

			MeshAnimation* activeAnimation;
			std::string anmNames[4];
			MeshAnimation* maleGuardAnimations[4];
			MeshAnimation* defaultAnimation;

			int animationStateCounter;

		public:
			int GetPlayerNum() const { return playerNum; }

			Vector3 GetPlayerForwardVector();
		};
	}
}

