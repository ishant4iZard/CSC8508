#pragma once
#include "Vector3.h"
using namespace NCL::Maths;

class Light
{
protected:
	Vector3 color;
	Vector3 pos;

public:
	Light(const Vector3& inColor)
	{
		color = inColor;
	}

	Light(const Vector3& inPos, const Vector3& inColor) {
		color = inColor;
		pos = inPos;
	}

	Vector3 GetColor()	{return color;}
	void	SetColor(const Vector3& inColor)	{ color = inColor; }

	Vector3 GetPos()	{return pos;}
	void	SetPos(const Vector3& inPos)		{ pos = inPos; }
};