#pragma once

#ifdef _WIN32
#include "../NCLCoreClasses/KeyboardMouseController.h"
#include "../CSC8503/UIWindows.h"
#include "OGLTextureManager.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#else
#include "GameTechRenderer.h"
#endif
#else// PROSPERO
#include "GameTechAGCRenderer.h"
#include "../CSC8503/UIPlaystation.h"
#include "PS5Controller.h"
#endif

#include "PhysicsSystem.h"
#include "AiTreeObject.h"
#include "AiStatemachineObject.h"
#include "BouncePad.h"
#include <vector>
#include "LevelEnum.h"
#include "LevelObjectEnum.h"
#include "TutorialGame.h"
#include "PowerUp.h"
#include "WindowsLevelLoader.h"
#include "ApplicationState.h"
#include "UIBase.h"

#include <chrono>
using namespace std::chrono;

#define USE_SHADOW = false
#define POWER_UP_SPAWN_TIME 30.0f
#define MAX_POWER_UP_COUNT 3
#define SAFE_DELETE_PBR_TEXTURE(a) for (uint8_t i = 0; i < (uint8_t)TextureType::MAX_TYPE; i++){ if (a[i] != NULL) delete a[i]; a[i] = NULL; }

enum class level {
	level1 = 1,
	level2 = 2,
};

namespace NCL {
	namespace CSC8503 {
		class PowerUp;
		class Player;
		class Goose;
		class BouncePad;
		class GravityWell;

		class TutorialGame : public EventListener		{
		public:
			TutorialGame();
			~TutorialGame();

			void BindEvents();
			void ReceiveEvent(EventType) override;

			virtual void UpdateGame(float dt);
			/*void UpdatePowerUpSpawnTimer(float dt);*/

			vector<GravityWell*> gravitywell;

			powerUpType getActivePowerup() {
				return activePowerUp;
			}
			void setActivePowerup(powerUpType inPowerup) {
				activePowerUp = inPowerup;
			}

			bool CloseGame = false;
			Texture* goldTextureList[(uint8_t)TextureType::MAX_TYPE];
			Shader* GetPbrShader() const { return pbrShader; }
			Mesh* GetSphereMesh() const { return sphereMesh; }
		protected:
			void InitialiseAssets();
			void InitCamera();
			void InitWorld();
			void InitTeleporters();
			PowerUp* InitPowerup();
			void InitNonePowerup(PowerUp* inPowerup, Shader* inShader);
			void InitIcePowerup(PowerUp* inPowerup, Shader* inShader);
			void InitSandPowerup(PowerUp* inPowerup, Shader* inShader);
			void InitWindPowerup(PowerUp* inPowerup, Shader* inShader);

			void SpawnDataDrivenLevel(GameLevelNumber inGameLevelNumber);
			
			void TestAddStaticObjectsToWorld();
			void AddPowerUpSpawnPoint(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);

			void SpawnWall(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);
			void SpawnFloor(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);
			void SpawnBouncingPad(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);
			void SpawnTarget(const Vector3 & inPosition, const Vector3 & inRotation, const Vector3 & inScale, const Vector2& inTiling);
			void SpawnBlackHole(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);
			void SpawnInvisibleWall(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size = Vector3(128,2,128));
			GameObject* AddObbCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, float elasticity = 0.81f);
			GameObject* AddAABBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, float elasticity = 0.81f);
			GameObject* AddCapsuleToWorld(const Vector3& position, float radius, float halfHeight, float inverseMass = 0.01f, float elasticity = 0.81f);
			GameObject* AddTeleporterToWorld(const Vector3& position1, const Vector3& position2, const Vector3& rotation1, const Vector3& rotation2, Vector3 dimensions, float inverseMass = 0.0f, float elasticity = 0.0f);

			GameObject* capsule;

#ifdef _WIN32
		KeyboardMouseController* controller;
		OGLTextureManager* bm;
#ifdef USEVULKAN
		GameTechVulkanRenderer* renderer;
#else
		GameTechRenderer* renderer;
#endif
#else
		GameTechAGCRenderer* renderer;
		PS5::PS5Controller* controller;
#endif

			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;

			Mesh*		capsuleMesh			= nullptr;
			Mesh*		cubeMesh			= nullptr;
			Mesh*		wallMesh			= nullptr;
			Mesh*		bouncePlatformMesh	= nullptr;
			Mesh*		sphereMesh			= nullptr;
			Mesh*		rebellionMeshChar	= nullptr;
			
			Texture*	basicTex		= nullptr;
			Texture*	sandTex			= nullptr;
			Texture*	portalTex		= nullptr;
			Texture*	blackholeTex	= nullptr;
			Texture*	targetTex		= nullptr;

			Shader*		basicShader		= nullptr;
			Shader*		pbrShader		= nullptr;
			Shader*		portalShader	= nullptr;
			Shader* 	instancePbrShader = nullptr;
			Shader*		blackholeShader = nullptr;
			Shader* 	targetholeShader = nullptr;
			Shader*		particleShader  = nullptr;
				
			Texture* groundTextureList[(uint8_t)TextureType::MAX_TYPE];
			Texture* wallTextureList[(uint8_t)TextureType::MAX_TYPE];
			Texture* sandTextureList[(uint8_t)TextureType::MAX_TYPE];
			Texture* lavaTextureList[(uint8_t)TextureType::MAX_TYPE];

			//Coursework Meshes
			Mesh*	charMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;
			Mesh*	gooseMesh	= nullptr;

			float timer;
			float finaltimer;

			GameObject* cube;
			GameObject* floor;

			level currentlevel;
			
			int score = 0;
			
			float v = 0, h = 0;

			float powerUpSpawnTimer = 0.0f;
			unsigned int activePowerUpCount = 0;
			powerUpType activePowerUp = powerUpType::none;

			WindowsLevelLoader* levelFileLoader;

			const int TIME_LIMIT = 200; // seconds

#pragma region Function Pointers
			typedef void (TutorialGame::*dataSpawnFunction) (const Vector3&, const Vector3&, const Vector3&, const Vector2&);
			dataSpawnFunction levelObjectSpawnFunctionList[static_cast<int>(LevelObjectEnum::MAX_OBJECT_TYPE)] = { &TutorialGame::SpawnWall , &TutorialGame::SpawnFloor , &TutorialGame::SpawnBouncingPad, &TutorialGame::SpawnTarget , &TutorialGame::SpawnBlackHole, &TutorialGame::AddPowerUpSpawnPoint, &TutorialGame::SpawnInvisibleWall };

			typedef void (TutorialGame::* powerupInitFunction) (PowerUp* inPowerup, Shader* inShader);
			powerupInitFunction powerupInitFunctionList[powerUpType::MAX_POWERUP] = { &TutorialGame::InitNonePowerup, &TutorialGame::InitIcePowerup , &TutorialGame::InitSandPowerup, &TutorialGame::InitWindPowerup };
#pragma endregion

			std::optional<microseconds> renderTimeCost;
			bool isDebuHUDActive = false;

			UIBase* ui;
			ApplicationState* appState;

			std::vector<Vector3> powerUpSpawnPointList;

			GameObject* teleporter1Display;
			GameObject* teleporter2Display;
		};
	}
}

