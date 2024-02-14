#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "AiTreeObject.h"
#include "BouncePad.h"

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

		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);



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

			void InitHole();
			void InitBouncePad();
			void InitLevelWall();

			void InitDefaultFloor();

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size = Vector3(128,2,128));

			AiTreeObject* AddAiToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity);
#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;
			AiTreeObject*		 aitreetest;

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

			bool gameover;
			bool gameWon;
			float timer;
			float finaltimer;
			bool serverStarted = false;

			level currentlevel;
			int score = 0;
			float v = 0, h = 0;
#pragma region BouncePad
			BouncePad* bouncePadList[5];
#pragma endregion
			const int TIME_LIMIT = 50;

		};
	}
}

