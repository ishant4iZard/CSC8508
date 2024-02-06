#pragma once
#include "GameObject.h"
#include "GameClient.h"
#include <iostream>
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
			void ReplenishProjectiles(float dt);

			bool isFire = false;
			void Fire();

			void AddScore(int score) {
				Score += score;
				std::cout << "Score: = " << Score;
			}
			void SetMovementDir(Vector3 dir) {
				movementDirection = dir;
			}

		protected:
			NetworkedGame* game;
			int playerNum;
			int Score = 0;
			float timeElapsed;
			float Oscillationspeed;
			Vector3 movementDirection;

			const float ORBIT_RADIUS = 75.0; // Ensure that this value matches the distance of the player from the center
			const float ORBIT_SPEED = 10.0f;
			const Vector3 ORBIT_CENTER = Vector3(0, 0, 0);

			int numProjectilesAccumulated;
			float projectileReplenishTimer;
			const int MAX_PROJECTILE_CAPACITY = 10;
			const int PROJECTILE_RELOAD_RATE = 1; // 1 projectile per second is replenished

		public:
			int GetPlayerNum() const { return playerNum; }

			Vector3 GetPlayerForwardVector();
		};
	}
}

