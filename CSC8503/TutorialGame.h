#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"


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
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			Player* GetPlayer() {
				return player;
			}


		protected:
			void InitialiseAssets();

			void InitCamera();
			//void UpdateKeys();

			void InitWorld();

			//void InitWorld2();

			/*void InitMaze();*/


			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			//void InitGameExamples();

			void InitCoin(int Amount, float radius);

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int length, int breadth, int height,  float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void BridgeConstraintTest();

			void InitOBBwall();
			void InitHole();

			void InitDefaultFloor();
			void InitAI();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			void InitInvisibleWall(Vector3 position, Vector3 size);
			void InitBoundary();

			void UpdateVoxels();


			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size = Vector3(128,2,128));
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f , bool isHollow = false, float elasticity = 0.81f);
			GameObject* AddKinematicSphereToWorld(const Vector3& position, float radius,  bool isHollow = false, float elasticity = 0.81f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, float elasticity = 0.81f);
			GameObject* AddObbCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, float elasticity =0.81f);
			GameObject* AddVoxelsToWorld(const Vector3& position, Vector3 dimensions,  float elasticity = 0.81f);
			GameObject* AddCapsuleToWorld(const Vector3& position, float radius, float halfHeight, float inverseMass = 10.0f, float elasticity = 0.81f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);
			Powerup* AddScoreToWorld(const Vector3& position, float radius, float inverseMass , int bonusScore = 1);
			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;


#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

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
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			std::vector<Powerup*> powerupList;
			std::vector<Goose*> EnemyList;
			std::vector<StateGameObject*> Enemy2List;
			std::vector<Voxels*> VoxelList;

			Player* player;
			bool gameover;
			bool gameWon;
			float timer;
			float finaltimer;

			GameObject* objClosest = nullptr;
			level currentlevel;
			int score = 0;
			float v = 0, h = 0;
		};
	}
}

