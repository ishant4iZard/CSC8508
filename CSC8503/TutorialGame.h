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
#else //_ORBIS
#include "../CSC8503/UIPlaystation.h"
#endif

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
			void GettestGameObjects(AiStatemachineObject *testGameObjects);

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
			
			GameObject* AddtestcubeToWorld(const Vector3& position1, const Vector3& position2, const Vector3& size);
			GameObject* testCube1;
			GameObject* testCube2;


			void InitHole();
			void InitGravityWell();
			void InitBouncePad();
			void InitLevelWall();
			void InitTeleporters();

			void InitDefaultFloor();
			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size = Vector3(128,2,128));

			void SpawnDataDrivenLevel(GameLevelNumber inGameLevelNumber);

			void SpawnWall(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale);
			void SpawnFloor(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale);
			void SpawnBouncingPad(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale);
			void SpawnTarget(const Vector3 & inPosition, const Vector3 & inRotation, const Vector3 & inScale);
			void SpawnBlackHole(const Vector3& inPosition, const Vector3& inRotation, const Vector3& inScale);



			GameObject* capsule;


			void InitAI();
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
			Mesh*	sphereMesh	= nullptr;

			Texture*	basicTex	= nullptr;
			Texture*	sandTex		= nullptr;
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	charMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;
			Mesh*	gooseMesh	= nullptr;

			//Coursework Additional functionality	

			float timer;
			float finaltimer;

			AiStatemachineObject * AddAiStateObjectToWorld(const Vector3& position);
			AiStatemachineObject* testStateObject;

			GameObject* cube;
			GameObject* floor1;

			vector<GameObject*> testGameObjects;

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
			typedef void (TutorialGame::*dataSpawnFunction) (const Vector3&, const Vector3&, const Vector3&);
			dataSpawnFunction levelObjectSpawnFunctionList[static_cast<int>(LevelObjectEnum::MAX_OBJECT_TYPE)] = { &TutorialGame::SpawnWall , &TutorialGame::SpawnFloor , &TutorialGame::SpawnBouncingPad, &TutorialGame::SpawnTarget , &TutorialGame::SpawnBlackHole };
#pragma endregion

#pragma region UI
			UIBase* ui;
#pragma endregion

			ApplicationState* appState;
		};
	}
}

