#pragma once
#include "Light.h"

class SpotLight :
    public Light
{
protected:
    Vector3 direction;
    /// <summary>
    /// Specifies the radius of this spot light
    /// </summary>
    float cutoffAngle;

    Vector3 position;
public:
    SpotLight(const Vector3& inPos, const Vector3& inDir, const Vector3& inColor, const float& inCutOffAngle) : Light(inColor)
    {
        position = inPos;
        direction = inDir;
        cutoffAngle = inCutOffAngle;
    }

    Vector3 GetDirection() { return direction; }
    float GetCutoffAngle() { return cutoffAngle; }

    Vector3 GetPosition() { return position; }
    void SetPosition(Vector3& inPos) { position = inPos; }
};