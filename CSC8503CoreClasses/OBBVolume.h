#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class OBBVolume : CollisionVolume
	{
	public:
		OBBVolume(const Maths::Vector3& halfDims, bool isTrigger = false,bool isKinematic =false) {
			type		= VolumeType::OBB;
			halfSizes	= halfDims;
			this->isTrigger = isTrigger;
			this->isKinematic = isKinematic;
		}
		~OBBVolume() {}

		Maths::Vector3 GetHalfDimensions() const {
			return halfSizes;
		}
	protected:
		Maths::Vector3 halfSizes;
	};
}

