#pragma once

#include "GameObject.h"

namespace NCL {
	namespace  CSC8503 {
		class NetworkPlayer;
		class NetworkedGame;

		class Projectile : public GameObject
		{
		public:
			static constexpr float FireForce = 20000;
			static constexpr float inverseMass = 1.0 / 10.0;

			static int CurrentAvailableProjectileID;

			Projectile(NetworkPlayer* Owner, NetworkedGame* Game);
			~Projectile();

			void OnCollisionBegin(GameObject* otherObject) override;

			NetworkPlayer* GetOwner() const { return Owner; }
			NetworkedGame* GetGame() const { return Game; }

			void ReduceTimeLeft(float dt) { timeLeft -= dt; }
			const float GetTimeLeft() { return timeLeft; }

		protected:
			NetworkedGame* Game;
			NetworkPlayer* Owner;
			float timeLeft;
			const int LIFE_TIME_AMOUNT = 5;
		};

	}
}
