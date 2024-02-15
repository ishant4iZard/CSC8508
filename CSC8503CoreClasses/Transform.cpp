#include "Transform.h"
#include <random>
using namespace NCL::CSC8503;

Transform::Transform()	{
	scale = Vector3(1, 1, 1);
}

Transform::~Transform()	{

}

void Transform::UpdateMatrix() {
	matrix =
		Matrix4::Translation(position) *
		Matrix4(orientation) *
		Matrix4::Scale(scale);
}

Transform& Transform::SetPosition(const Vector3& worldPos) {
	position = worldPos;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetScale(const Vector3& worldScale) {
	scale = worldScale;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetOrientation(const Quaternion& worldOrientation) {
	orientation = worldOrientation;
	UpdateMatrix();
	return *this;
}

Transform& Transform::RandomPosition(const Vector3& worldPos,bool openorclose)
{
	if (openorclose)
	{
		float offsetX = randomFloat(minX, maxX);
		//float offsetY = randomFloat(minY, maxY);
		float offsetZ = randomFloat(minZ, maxZ);

		position =  Vector3(offsetX, 0, offsetZ);
		
	}
	else {
		position = worldPos;
	}
	UpdateMatrix();
	return *this;
}

//Transform& Transform::SetRandomOrientation() {
//
//	Quaternion randomOrientation = Transform::GenerateRandomRotation();
//
//	orientation = randomOrientation;
//
//	UpdateMatrix();
//
//	return *this;
//}

float Transform::randomFloat(float min, float max) {
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

//Quaternion Transform::GenerateRandomRotation()
//{
//	std::random_device rd;
//	std::mt19937 gen(rd());
//	std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
//
//
//	float angleX = dis(gen) * 180.0f;
//	//float angleY = dis(gen) * 180.0f;
//	float angleZ = dis(gen) * 180.0f;
//
//	Quaternion quaternionX(Vector3::UnitX, angleX);
//	//Quaternion quaternionY(Vector3::UnitY, angleY);
//	Quaternion quaternionZ(Vector3::UnitZ, angleZ);
//
//	Quaternion randomRotation = quaternionX * quaternionY * quaternionZ;
//
//	return randomRotation;
//}