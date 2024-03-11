#pragma once

#include "GameObject.h"

#include "Mesh.h"
#include "MeshMaterial.h"
#include "MeshAnimation.h"

#include <string>
using namespace NCL::Rendering;

namespace NCL {
	namespace  CSC8503 {

		//const int MAX_ANM2nd = 32;

		enum AnmName2nd
		{
			IDLE_MAX,
			SINGLESHOT,
			WALK,
			RUN,

			NUM_ANMS2nd
		};

		class MaxGuard : public GameObject {
		public:
			MaxGuard(const std::string& objectName);
			~MaxGuard() {};

			void SetMesh(Mesh* newMesh) { maxGuardMesh = newMesh; }
			void SetMaterial();
			void SetAnimation(int index) { nextAnimation = maxGuardAnimations[index]; activeAnimation = maxGuardAnimations[index]; }
			MeshAnimation* GetAnimation() { return activeAnimation; }
			MeshMaterial* GetMaterial() { return maxGuardMaterial; }
			std::string GetAnimationName(int index) { return anmNames[index]; }

			std::string GetName() { return maxGuardName; }

			void SetAnimatedObjectID(int newID) {
				animatedObjectID = newID;
			}

			int GetAnimatedObjectID() {
				return animatedObjectID;
			}

		protected:
			Mesh* maxGuardMesh;
			MeshMaterial* maxGuardMaterial;
			MeshAnimation* maxGuardAnimations[4];
			MeshAnimation* activeAnimation;
			MeshAnimation* nextAnimation;
			std::string anmNames[4];

			std::string maxGuardName;

			int animatedObjectID;
		};
	}
}