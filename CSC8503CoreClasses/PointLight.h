#pragma once
#include "Light.h"

class PointLight :
    public Light
{
protected:
    /// <summary>
    /// X : Constant
    /// Y : Linear
    /// Z : Quadratic
    /// </summary>
    Vector3 constantLinearQuadratic;
    Vector3 position;
public:
    PointLight(const Vector3& inPos, const Vector3& inColor);
    /// <summary>
    /// 
    /// </summary>
    /// <param name="inPos">World position of light</param>
    /// <param name="inColor">Color of light</param>
    /// <param name="inConstant">Constant</param>
    /// <param name="inLinear">Linear</param>
    /// <param name="inQuadratic">Quadratic</param>
    PointLight(const Vector3& inPos, const Vector3& inColor,
        const float& inConstant, const float& inLinear, const float& inQuadratic);
    float GetConstant() { return constantLinearQuadratic.x; }
    float GetLinear() { return constantLinearQuadratic.y; }
    float GetQuadratic() { return constantLinearQuadratic.z; }
    Vector3 GetConstnatLinearQuadratic() { return constantLinearQuadratic; }
};