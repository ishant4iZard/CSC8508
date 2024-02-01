#include "PointLight.h"

PointLight::PointLight(const Vector3& inPos, const Vector3& inColor) : Light(inColor)
{
	position = inPos;
	color = inColor;
}

PointLight::PointLight(const Vector3& inPos, const Vector3& inColor, const float& inConstant, const float& inLinear, const float& inQuadratic)
	: Light(inColor)
{
	position = inPos;
	constantLinearQuadratic.x = inConstant;
	constantLinearQuadratic.y = inLinear;
	constantLinearQuadratic.z = inQuadratic;
}