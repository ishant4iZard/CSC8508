#pragma once
#include "GameObject.h"

namespace NCL {
	namespace  CSC8503 {
		enum powerUpType {
			none,
			ice,
			sand,
			wind
		};
		class PowerUp : public GameObject
		{
		public:
			PowerUp();
			~PowerUp() {}

			void setPowerup(powerUpType inPowerup) {
				PowerUpType = inPowerup;
			}

			powerUpType getPowerUp() {
				return PowerUpType;
			}

			void OnTriggerBegin(GameObject* otherObject)override;

		private:
			powerUpType PowerUpType;
		};
	}
}