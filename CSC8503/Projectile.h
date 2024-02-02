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

		protected:
			NetworkedGame* Game;
			NetworkPlayer* Owner;
		};

	}
}
