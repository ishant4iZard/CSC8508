#pragma once
#include"GameObject.h"
#include"GameTechRenderer.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame;
		class Voxels :public GameObject
		{
		public:
			Voxels(const std::string& objectName = "" );
			~Voxels();

			virtual void OnCollisionBegin(GameObject* otherObject) override;

			bool Destroyed() {
				return isDestroyed;
			}

		protected:

			Vector3 defaultPos;
			Vector3 defaultRot;
			Vector4 defaultColor;
			float defaultInvMass;
			bool isDestroyed ;

			Vector3 basePos;
		};
	}
}
