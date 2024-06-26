#pragma once
#include "Light.h"
namespace NCL
{
    namespace CSC8503
    {
        class DirectionalLight : public Light
        {
        protected:
            NCL::Maths::Vector3 direction;
            float intensity;
        public:
            DirectionalLight(const NCL::Maths::Vector3& inDir, const float& inIntensity, const NCL::Maths::Vector4& inColor) :
                Light(inColor)
            {
                direction = inDir;
                intensity = inIntensity;
            }

            const NCL::Maths::Vector3& GetDirection() { return direction; }

            float GetIntensity() { return intensity; }
            void SetDirection(const NCL::Maths::Vector3& inDir) { direction = inDir; }
            void SetIntensity(float inIntensity) { intensity = inIntensity; }
        };
    }
}