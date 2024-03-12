#pragma once
#include "PhysicsSystem.h"
#include "MeshAnimation.h"
#include "GameTechRendererInterface.h"
#include "NetworkPlayer.h"
#include "AudioEngine.h"

namespace NCL {
	namespace CSC8503 {
		class PS5_Game {
		public:
			PS5_Game(GameWorld& gameWorld, GameTechRendererInterface& renderer, PhysicsSystem& physics);
			~PS5_Game();

			virtual void UpdateGame(float dt);

			void SpawnProjectile(NetworkPlayer* player, Vector3 firePos, Vector3 fireDir) {};

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on).
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			GameWorld& world;
			GameTechRendererInterface& renderer;
			PhysicsSystem& physics;

			Controller* controller;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh* capsuleMesh = nullptr;
			Mesh* cubeMesh = nullptr;
			Mesh* sphereMesh = nullptr;

			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;

			//Coursework Meshes
			Mesh* charMesh = nullptr;
			Mesh* enemyMesh = nullptr;
			Mesh* bonusMesh = nullptr;

			MeshAnimation* enemyAnim = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			Vector3 lockedOffset = Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;

			AudioEngine* audioEngine;
		};
	}
}

