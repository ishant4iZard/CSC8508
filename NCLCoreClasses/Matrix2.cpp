/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Matrix2.h"
#include "Maths.h"

using namespace NCL;
using namespace NCL::Maths;

Matrix2::Matrix2(void)	{
	array[0][0] = 1.0f;
	array[0][1] = 0.0f;
	array[1][0] = 0.0f;
	array[1][1] = 1.0f;
}

Matrix2::Matrix2(float elements[4]) {
	array[0][0] = elements[0];
	array[0][1] = elements[1];
	array[1][0] = elements[2];
	array[1][1] = elements[3];
}

Matrix2::Matrix2(const Vector2& colA, const Vector2& colB) {
	array[0][0] = colA[0];
	array[0][1] = colA[1];
	array[1][0] = colB[0];
	array[1][1] = colB[1];
}

void Matrix2::ToZero() {
	array[0][0] = 0.0f;
	array[0][1] = 0.0f;
	array[1][0] = 0.0f;
	array[1][1] = 0.0f;
}

Matrix2 Matrix2::Rotation(float degrees)	{
	Matrix2 mat;

	float radians = Maths::DegreesToRadians(degrees);
	float s = sin(radians);
	float c = cos(radians);

	mat.array[0][0] = c;
	mat.array[0][1] = s;
	mat.array[1][0] = -s;
	mat.array[1][1] = c;

	return mat;
}

Matrix2& Matrix2::SetRow(unsigned int row, const Vector2& val) {
	assert(row < 2);
	array[0][row] = val.x;
	array[1][row] = val.y;
	return *this;
}

Matrix2& Matrix2::SetColumn(unsigned int column, const Vector2& val) {
	assert(column < 2);
	array[column][0] = val.x;
	array[column][1] = val.y;
	return *this;
}

Vector2 Matrix2::GetRow(unsigned int row) const {
	assert(row < 2);
	return Vector2(array[0][row], array[1][row]);
}

Vector2 Matrix2::GetColumn(unsigned int column) const {
	assert(column < 2);
	return Vector2(array[column][0], array[column][1]);
}

Vector2 Matrix2::GetDiagonal() const {
	return Vector2(array[0][0], array[1][1]);
}

void	Matrix2::SetDiagonal(const Vector2& in) {
	array[0][0] = in.x;
	array[1][1] = in.y;
}

Vector2 Matrix2::operator*(const Vector2& v) const {
	Vector2 vec;

	vec.x = v.x * array[0][0] + v.y * array[1][0];
	vec.y = v.x * array[0][1] + v.y * array[1][1];

	return vec;
};

//This is going to assume that the matrix is actually invertable!
void Matrix2::Invert() {
	float determinant = (array[0][0] * array[1][1]) - (array[0][1] * array[1][0]);
	float invDet = 1.0f / determinant; //Turn our divides into multiplies!

	Matrix2 temp = *this;

	array[0][0] =  temp.array[1][1] * invDet;
	array[0][1] = -temp.array[1][0] * invDet;
	array[1][0] = -temp.array[0][1] * invDet;
	array[1][1] =  temp.array[0][0] * invDet;
}

Matrix2 Matrix2::Inverse() const {
	Matrix2 newMatrix = *this;
	newMatrix.Invert();
	return newMatrix;
}