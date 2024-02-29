#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "AiTreeObject.h"
#include "AiStatemachineObject.h"
#include "BouncePad.h"
#include <vector>
#include "LevelEnum.h"
#include "LevelObjectEnum.h"
#include "TutorialGame.h"

#ifdef _WIN32
#include "WindowsLevelLoader.h"
#endif // _WIN32

#include "ApplicationState.h"
#include "../CSC8503/UIBase.h"
#ifdef _WIN32
#include "../CSC8503/UIWindows.h"
#include "OGLTextureManager.h"
#else //_ORBIS
#include "../CSC8503/UIPlaystation.h"
#endif

#define USE_SHADOW = false
enum class level {
	level1 = 1,
	level2 = 2,
};

namespace NCL {
	namespace CSC8503 {
		class Powerup;
		class Player;
		class Goose;
		class Voxels;
		class BouncePad;
		class GravityWell;

		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			GravityWell* gravitywell;

		protected:
			void InitialiseAssets();

			void InitCamera();

			void InitWorld();


			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/

			GameObject* AddObbCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, float elasticity = 0.81f);
			GameObject* AddAABBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, float elasticity = 0.81f);
			GameObject* AddCapsuleToWorld(const Vector3& position, float radius, float halfHeight, float inverseMass = 0.01f, float elasticity = 0.81f);
			GameObject* AddTeleporterToWorld(const Vector3& position1, const Vector3& position2, const Vector3& rotation1, const Vector3& rotation2, Vector3 dimensions, float inverseMass = 0.0f, float elasticity = 0.0f);
			


			void InitHole();
			void InitGravityWell();
			void InitBouncePad();
			void InitLevelWall();
			void InitTeleporters();
			void TestAddStaticObjectsToWorld();

			void InitDefaultFloor();
			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size = Vector3(128,2,128));

			void SpawnDataDrivenLevel(GameLevelNumber inGameLevelNumber);

			void SpawnWall(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);
			void SpawnFloor(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);
			void SpawnBouncingPad(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);
			void SpawnTarget(const Vector3 & inPosition, const Vector3 & inRotation, const Vector3 & inScale, const Vector2& inTiling);
			void SpawnBlackHole(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale, const Vector2& inTiling);



			GameObject* capsule;


			//void InitDefaultFloor();
			void ProcessFrameAddresses();
			void ObjectRay(GameObject* gameObject, const std::vector<GameObject*>& gameObjects);

			//GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size = Vector3(128,2,128));

			AiTreeObject* AddAiToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity);
#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;
			AiTreeObject*		 aitreetest;
			vector<Vector3> frameAddresses;
			Vector3 aichaseposition;

			KeyboardMouseController controller;

			bool useGravity;


			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh* wallMesh = nullptr;
			Mesh* bouncePlatformMesh = nullptr;
			Mesh*	sphereMesh	= nullptr;

			Texture*	basicTex	= nullptr;
			Texture*	sandTex		= nullptr;
			Shader*		basicShader = nullptr;
			Shader* pbrShader = nullptr;
			Shader* instancePbrShader = nullptr;

			Texture* groundTextureList[(uint8_t)TextureType::MAX_TYPE];
			Texture* wallTextureList[(uint8_t)TextureType::MAX_TYPE];
			Texture* sandTextureList[(uint8_t)TextureType::MAX_TYPE];

			//Coursework Meshes
			Mesh*	charMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;
			Mesh*	gooseMesh	= nullptr;

			//Coursework Additional functionality	

			float timer;
			float finaltimer;

			GameObject* cube;
			GameObject* floor;
			level currentlevel;
			int score = 0;
			float v = 0, h = 0;


#pragma region BouncePad
			BouncePad* bouncePadList[5];
#pragma endregion

#pragma region PlaceholderAI
			std::vector<GameObject*> placeHolderAIs;
			void InitPlaceholderAIs();
#pragma endregion

#ifdef _WIN32
			WindowsLevelLoader* levelFileLoader;
#endif // _WIN32


			const int TIME_LIMIT = 200;

#pragma region Function Pointers
			typedef void (TutorialGame::*dataSpawnFunction) (const Vector3&, const Vector3&, const Vector3&, const Vector2&);
			dataSpawnFunction levelObjectSpawnFunctionList[static_cast<int>(LevelObjectEnum::MAX_OBJECT_TYPE)] = { &TutorialGame::SpawnWall , &TutorialGame::SpawnFloor , &TutorialGame::SpawnBouncingPad, &TutorialGame::SpawnTarget , &TutorialGame::SpawnBlackHole };
#pragma endregion

#pragma region UI
			UIBase* ui;
#pragma endregion

			ApplicationState* appState;
			OGLTextureManager* bm;
		};
	}
}

