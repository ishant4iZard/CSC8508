#pragma once
#include "CollisionVolume.h"
#include "Vector3.h"

namespace NCL {
	using namespace NCL::Maths;
	class AABBVolume : CollisionVolume
	{
	public:
		AABBVolume(const Vector3& halfDims, bool isTrigger = false, bool isKinematic = false) {
			type		= VolumeType::AABB;
			halfSizes	= halfDims;
			this->isTrigger = isTrigger;
			this->isKinematic = isKinematic;
		}
		~AABBVolume() {

		}

		Vector3 GetHalfDimensions() const {
			return halfSizes;
		}

	protected:
		Vector3 halfSizes;
	};
}
