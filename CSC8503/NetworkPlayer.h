#pragma once
#include "GameObject.h"
#include "GameClient.h"

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

			bool isFire = false;
			void Fire();

			void SetMovementDir(Vector3 dir) {
				movementDirection = dir;
			}

		protected:
			NetworkedGame* game;
			int playerNum;
			float timeElapsed;
			float Oscillationspeed;
			Vector3 movementDirection;

			const float ORBIT_RADIUS = 75.0; // Ensure that this value matches the distance of the player from the center
			const float ORBIT_SPEED = 10.0f;
			const Vector3 ORBIT_CENTER = Vector3(0, 0, 0);

		public:
			int GetPlayerNum() const { return playerNum; }

			Vector3 GetPlayerForwardVector();
		};
	}
}

