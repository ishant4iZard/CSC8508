#pragma once
#include "Vector4.h"
#include "Vector3.h"
using namespace NCL::Maths;

class Light
{
protected:
	Vector4 color;
	Vector3 position;
	float radius;

public:
	Light() {} // Default constructor , we � ll be needing this later !
	Light(const Vector3 & position, const Vector4 & color, float radius) {
	this -> position = position;
	this -> color = color;
	this -> radius = radius;
	}
	Light(const Vector3& color) {
		this->color = color;
	}
	Light(const Vector3& position, const Vector4& color) {
		this->position = position;
		this->color = color;
	}
	~Light(void) {};

	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3 & val) { position = val; }

	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }

	Vector4 GetColour() const { return color; }
	void SetColour(const Vector4 & val) { color = val; }

};