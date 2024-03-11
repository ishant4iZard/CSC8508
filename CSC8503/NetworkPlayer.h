#pragma once
#include "GameObject.h"
#include "GameClient.h"
#include <iostream>
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

		public:
			int GetPlayerNum() const { return playerNum; }

			Vector3 GetPlayerForwardVector();
		};
	}
}

