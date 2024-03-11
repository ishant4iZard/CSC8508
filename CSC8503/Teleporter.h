#pragma once
#include "GameObject.h"
#include "Vector3.h"

namespace NCL {
	namespace  CSC8503 {

		class Teleporter : public GameObject
		{
		public:

			Teleporter();
			Teleporter(Teleporter* otherTeleporter);
			~Teleporter();

			void OnTriggerBegin(GameObject* otherObject) override;
			void OnTriggerEnd(GameObject* otherObject) override;

			void setConnectedTeleporter(Teleporter* other) {
				connectedTeleporter = other;
			}

			Maths::Vector3 getOutNormal() {
				return outNormal;
			}

			void setOutNormal(Maths::Vector3 normal) {
				outNormal = normal;
			}

		protected:
			Teleporter*		connectedTeleporter;
			Maths::Vector3	outNormal;
		};

	}
}