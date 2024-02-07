#pragma once
#include <vector>
#include "GameObject.h"

namespace NCL {
	namespace  CSC8503 {
		class Projectile;

		class GravityWell : public GameObject
		{
		public:

			GravityWell();
			~GravityWell() {};

			void SetGravityRange(float gravityRange) {
				GravityRange = gravityRange;
			}
			void SetGravityForce(float gravityForce) {
				GravityRange = gravityForce;
			}


			void PullProjectilesWithinField(std::vector<Projectile*> projectileList);

		protected:

			void AddGravityForce(Projectile* projectile);
			bool IsWithinRange(Projectile* projectile);

			float GravityForce;
			float GravityRange;

		};

	}
}
