#pragma once
#include "PhysicsSystem.h"
#include "MeshAnimation.h"
#include "GameTechRendererInterface.h"
#include "NetworkPlayer.h"
#include "AudioEngine.h"
#include "TutorialGame.h"

namespace NCL {
	namespace CSC8503 {
		class PS5_Game : public TutorialGame {
		public:
			PS5_Game();
			~PS5_Game();

			void UpdateGame(float dt) override;

			void SpawnProjectile(NetworkPlayer* player, Vector3 firePos, Vector3 fireDir);
			void SpawnPlayer();

			void StartLevel();
			void EndLevel();

		protected:

			NetworkPlayer* player;

			void MovePlayer(float dt);
		};
	}
}

