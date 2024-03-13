#pragma once
#include "PhysicsSystem.h"
#include "MeshAnimation.h"
#include "GameTechRendererInterface.h"
#include "NetworkPlayer.h"
#include "AudioEngine.h"
#include "TutorialGame.h"
#include "Projectile.h"
#include <vector>

namespace NCL {
	namespace CSC8503 {
		class PS5_Game : public TutorialGame {
		public:
			PS5_Game();
			~PS5_Game();

			void UpdateGame(float dt) override;


			void StartLevel();
			void EndLevel();
			void SpawnProjectile(NetworkPlayer* player, Vector3 firePos, Vector3 fireDir);

		protected:
			void SpawnPlayer();

			NetworkPlayer* player;

			void MovePlayer(float dt);
			void Fire();

			const int PROJECTILE_POOL_SIZE = 60;
			std::vector<Projectile*> projectileList;
			void InitializeProjectilePool();

			const float FIRE_RATE = 1;
			float timeSinceFire = 0;
		};
	}
}

