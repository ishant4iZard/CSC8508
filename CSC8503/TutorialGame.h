#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
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



			void InitHole();

			void InitDefaultFloor();

			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& size = Vector3(128,2,128));


#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

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

