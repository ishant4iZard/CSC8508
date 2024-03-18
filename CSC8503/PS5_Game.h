#pragma once
#include "AiStatemachineObject.h"
#include "PhysicsSystem.h"
#include "MeshAnimation.h"
#include "GameTechRendererInterface.h"
#include "NetworkPlayer.h"
#include "AudioEngine.h"
#include "TutorialGame.h"
#include "Projectile.h"
#include "UIPlaystation.h"
#include "UIBase.h"
#include "ApplicationState.h"
#include "MeshAnimation.h"
#include "DebugHUD.h"

#include <vector>
#include <chrono>
using namespace std::chrono;

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

			PS5::PS5Controller* GetController() const { return controller; }

		protected:
			void InitialisePlayerAssets();
			void SpawnPlayer();

			NetworkPlayer*	player					= nullptr;
			Mesh*			playerMesh				= nullptr;
			MeshAnimation*	playerWalkingAnimation	= nullptr;
			Shader*			playerShader			= nullptr;


			void MovePlayer(float dt);
			void Fire();

			const int PROJECTILE_POOL_SIZE = 20;
			std::vector<Projectile*> projectileList;
			void InitializeProjectilePool();

			const float FIRE_RATE = 1.5;
			float timeSinceFire = 0;

			void SpawnAI();
			AiStatemachineObject* AddAiStateObjectToWorld(const Vector3& position);
			AiStatemachineObject* AIStateObject;

			UIBase* ui;

			const int GAME_TIME_LIMIT = 200; // seconds
			float timeElapsed = 0;
			ApplicationState* appState;

			DebugHUD* debugHUD;
			bool isDebuHUDActive;

			AudioEngine* audioEngine;

			void SwitchAnimations(RenderObject* renderObject, MeshAnimation* animation);
			class PS5MenuSystem* Menu;

		public:
			inline float GetTimeLeft() const { return GAME_TIME_LIMIT - timeElapsed; }
			int GetPlayerScore() const;
			int GEtPlayerBulletsNum() const;
			powerUpType GetActivatedPowerUpType() const { return physics->GetCurrentPowerUpState(); }
		};
	}
}

