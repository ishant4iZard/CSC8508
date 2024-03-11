#pragma once

#include "GameObject.h"

namespace NCL {
	namespace  CSC8503 {
		class NetworkPlayer;
		class NetworkedGame;

		class Projectile : public GameObject
		{
		public:
			static constexpr float FireForce = 1000;
			static constexpr float inverseMass = 1.0 / 10.0;

			static int CurrentAvailableProjectileID;

			Projectile(NetworkPlayer* Owner, NetworkedGame* Game);
			Projectile(Vector3* proPositions);
			~Projectile();

			void OnCollisionBegin(GameObject* otherObject) override;

			NetworkPlayer* GetOwner() const { return Owner; }
			NetworkedGame* GetGame() const { return Game; }

			void ReduceTimeLeft(float dt) { timeLeft -= dt; }
			const float GetTimeLeft() { return timeLeft; }

			bool getTeleport() {
				return canTeleport;
			}

			void setTeleport(bool teleport) {
				canTeleport = teleport;
			}

		protected:
			NetworkedGame* Game;
			NetworkPlayer* Owner;
			float timeLeft;
			bool canTeleport = true;
			const int LIFE_TIME_AMOUNT = 15;
			GameObject* projectileToChase;
		};

	}
}
