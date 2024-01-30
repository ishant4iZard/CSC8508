/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Vector4i.h"
#include "Vector2i.h"
#include "Vector3i.h"

using namespace NCL;
using namespace Maths;

Vector4i::Vector4i(const Vector3i& v3, int newW) : x(v3.x), y(v3.y), z(v3.z), w (newW)  {

}

Vector4i::Vector4i(const Vector2i& v2, int newZ, int newW) : x(v2.x), y(v2.y), z(newZ), w(newW) {

}

constexpr Vector4i Vector4i::Clamp(const Vector4i& input, const Vector4i& mins, const Vector4i& maxs) {
	return Vector4i(
		std::clamp(input.x, mins.x, maxs.x),
		std::clamp(input.y, mins.y, maxs.y),
		std::clamp(input.z, mins.z, maxs.z),
		std::clamp(input.w, mins.w, maxs.w)
	);
}