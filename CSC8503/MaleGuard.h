#pragma once

#include "GameObject.h"

#include "Mesh.h"
#include "MeshMaterial.h"
#include "MeshAnimation.h"

#include <string>
using namespace NCL::Rendering;

namespace NCL {
	namespace  CSC8503 {

		const int MAX_ANM = 32;

		enum AnmName
		{
			MALEGUARD_IDLE,
			MALEGUARD_GUNFIRE,
			MALEGUARD_STEPFORWARD,
			MALEGUARD_HAPPY,

			MAXGUARD_IDLE,
			MAXGUARD_GUNFIRE,
			MAXGUARD_WALK,
			MAXGUARD_INCENTIVISE,

			NUM_ANMS
		};

		class MaleGuard : public GameObject {
		public:
			MaleGuard(const std::string& objectName);
			~MaleGuard() {};

			void SetMesh(Mesh* newMesh) { maleGuardMesh = newMesh; }
			void SetMaterial();
			void SetAnimation(int index) { nextAnimation = maleGuardAnimations[index]; activeAnimation = maleGuardAnimations[index]; }
			MeshAnimation* GetAnimation() { return activeAnimation; }
			MeshMaterial* GetMaterial() { return maleGuardMaterial; }
			string GetAnimationName(int index) { return anmNames[index]; }
			
			string GetName() { return maleGuardName; }

			void SetAnimatedObjectID(int newID) {
				animatedObjectID = newID;
			}

			int GetAnimatedObjectID() {
				return animatedObjectID;
			}

		protected:
			Mesh* maleGuardMesh;
			MeshMaterial* maleGuardMaterial;
			MeshAnimation* maleGuardAnimations[4];
			MeshAnimation* activeAnimation;
			MeshAnimation* nextAnimation;
			std::string anmNames[4];

			string maleGuardName;

			int animatedObjectID;
		};
	}
}