/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Vector4i.h"
#include "Vector3i.h"
#include "Vector2i.h"

using namespace NCL;
using namespace Maths;


Vector3i::Vector3i(const Vector2i& v2, int newZ) : x(v2.x), y(v2.y), z(newZ) {
}

Vector3i::Vector3i(const Vector4i& v4) : x(v4.x), y(v4.y), z(v4.z) {
}

constexpr Vector3i Vector3i::Clamp(const Vector3i& input, const Vector3i& mins, const Vector3i& maxs) {
	return Vector3i(
		std::clamp(input.x, mins.x, maxs.x),
		std::clamp(input.y, mins.y, maxs.y),
		std::clamp(input.z, mins.z, maxs.z)
	);
}