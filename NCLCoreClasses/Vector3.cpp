/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Vector3.h"
#include "Vector2.h"
#include "Vector4.h"

using namespace NCL;
using namespace Maths;


Vector3::Vector3(const Vector2& v2, float newZ) : x(v2.x), y(v2.y), z(newZ) {
}

Vector3::Vector3(const Vector4& v4) : x(v4.x), y(v4.y), z(v4.z) {
}

Vector3 Vector3::Clamp(const Vector3& input, const Vector3& mins, const Vector3& maxs) {
	return Vector3(
		std::clamp(input.x, mins.x, maxs.x),
		std::clamp(input.y, mins.y, maxs.y),
		std::clamp(input.z, mins.z, maxs.z)
	);
}