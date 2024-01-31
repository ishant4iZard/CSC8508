#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class SphereVolume : CollisionVolume
	{
	public:
		SphereVolume(float sphereRadius = 1.0f, bool isTrigger = false,bool isKinematic = false) {
			type	= VolumeType::Sphere;
			radius	= sphereRadius;
			this->isTrigger = isTrigger;
			this->isKinematic = isKinematic;
		}
		~SphereVolume() {}

		float GetRadius() const {
			return radius;
		}
	protected:
		float	radius;
	};
}

