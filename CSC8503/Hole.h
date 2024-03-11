#pragma once
// TODO can be corrected after player FIX

#include "GameObject.h"

namespace NCL {
	namespace  CSC8503 {
		class NetworkPlayer;
		class NetworkedGame;

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
