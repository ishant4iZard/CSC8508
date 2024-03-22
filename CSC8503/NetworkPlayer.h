#pragma once
#include "GameObject.h"
#include "GameClient.h"
#include <iostream>

#include "RenderObject.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame;

		class NetworkPlayer : public GameObject {
		public:
			NetworkPlayer(TutorialGame* game, int num);
			~NetworkPlayer();

			void OnCollisionBegin(GameObject* otherObject) override;

			void SetPlayerYaw(const Vector3& pointPos);

			void OscillatePlayer(float dt);
			void RotatePlayer(float dt);
			void MovePlayerInSquarePattern(float dt); // The player will move along the boundaries of the level in a square pattern
			void MovePlayerTowardsCursor(float dt);
			void RotatePlayerBasedOnController(float dt, float rotationX, float rotationY);
			void MovePlayerBasedOnController(float dt, float horizontalInput, float verticalInput);
			void ReplenishProjectiles(float dt);

			bool isFire = false;
			void Fire();

			void AddScore(int score) {
				Score += score;
			}

			int GetScore() {
				return Score;
			}

			void SetMovementDir(Vector3 dir) {
				movementDirection = dir;
			}

			int GetNumBullets() {
				return numProjectilesAccumulated;
			}

			void InitAnimation(const AnimationType& inType, MeshAnimation* inAnimation) {
				if (inType != AnimationType::MAX_ANM) {
					animations[(uint8_t)inType] = inAnimation;
				}
			}

		protected:
			TutorialGame* game;
			int playerNum;
			int Score = 0;
			float timeElapsed;
			float movementSpeed;
			Vector3 movementDirection;
			Vector3 pointPos;

			const int MAX_PROJECTILE_CAPACITY = 10;
			int numProjectilesAccumulated;
			float projectileReplenishTimer;

			MeshAnimation* animations[(uint8_t)AnimationType::MAX_ANM];

		public:
			int GetPlayerNum() const { return playerNum; }

			Vector3 GetPlayerForwardVector();
			Vector3 GetPlayerBackwardVector();
		};
	}
}

