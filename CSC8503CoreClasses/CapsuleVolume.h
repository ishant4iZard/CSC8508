#pragma once
#include "CollisionVolume.h"

namespace NCL {
    class CapsuleVolume : public CollisionVolume
    {
    public:
        CapsuleVolume(float halfHeight, float radius,bool isTrigger = false,bool isKinematic = false) {
            this->halfHeight    = halfHeight;
            this->radius        = radius;
            this->type          = VolumeType::Capsule;
            this->isTrigger     = isTrigger;
            this->isKinematic = isKinematic;
        };
        ~CapsuleVolume() {

        }
        float GetRadius() const {
            return radius;
        }

        float GetHalfHeight() const {
            return halfHeight;
        }

    protected:
        float radius;
        float halfHeight;
    };
}

