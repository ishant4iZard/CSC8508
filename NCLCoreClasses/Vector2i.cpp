/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Vector2i.h"
#include "Vector3i.h"
#include "Vector4i.h"

using namespace NCL;
using namespace Maths;

Vector2i::Vector2i(const Vector3i& v3) : x(v3.x), y(v3.y){
}

Vector2i::Vector2i(const Vector4i& v4) : x(v4.x), y(v4.y) {
}

constexpr Vector2i Vector2i::Clamp(const Vector2i& input, const Vector2i& mins, const Vector2i& maxs) {
	return Vector2i(
		std::clamp(input.x, mins.x, maxs.x),
		std::clamp(input.y, mins.y, maxs.y)
	);
}