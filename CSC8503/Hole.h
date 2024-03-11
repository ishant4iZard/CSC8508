#pragma once
#include "GameObject.h"

namespace NCL {
	namespace  CSC8503 {
		class Hole : public GameObject
		{
		public:
			Hole();
			~Hole();

			void OnCollisionBegin(GameObject* otherObject) override;

		protected:
		};

	}
}
