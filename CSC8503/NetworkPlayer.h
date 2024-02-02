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

			bool isFire = false;
			void Fire();

			void AddScore(int score) {
				Score += score;
				std::cout << "Score: = " << Score;
			}

		protected:
			NetworkedGame* game;
			int playerNum;
			int Score = 0;

		public:
			int GetPlayerNum() const { return playerNum; }

			Vector3 GetPlayerForwardVector();
		};
	}
}

