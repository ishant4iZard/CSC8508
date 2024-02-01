#pragma once
#include "Light.h"
#include <utility>

class DirectionalLight :
    public Light
{
protected:
    Vector3 direction;
    float intensity;
public:
    DirectionalLight(const Vector3& inDir, const float& inIntensity, const Vector3& inPos, const Vector3& inColor) : Light(inPos, inColor)
    {
        direction = inDir;
        intensity = std::max(0.0f, std::min(inIntensity, 1.0f));
    }

    Vector3 GetDirection() { return direction; }
    /// <summary>
    /// Returns specular intensity of the light.
    /// </summary>
    /// <returns></returns>
    float GetIntensity() { return intensity; }
};