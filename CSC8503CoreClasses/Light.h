#pragma once
#include "Vector3.h"
using namespace NCL::Maths;

class Light
{
protected:
	Vector3 color;
public:
	Light(const Vector3& inColor)
	{
		color = inColor;
	}

	Vector3 GetColor() { return color; }
	void SetColor(const Vector3& inColor) { color = inColor; }
};