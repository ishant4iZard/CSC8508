/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Vector2.h"

namespace NCL::Maths {
	class Matrix2 {
	public: 
		float	array[2][2];
	public:
		Matrix2(void);
		Matrix2(float elements[4]);

		Matrix2(const Vector2& colA, const Vector2& colB);

		~Matrix2(void) = default;

		void ToZero();

		Matrix2& SetRow(unsigned int row, const Vector2& val);
		Matrix2& SetColumn(unsigned int column, const Vector2& val);

		Vector2 GetRow(unsigned int row) const;
		Vector2 GetColumn(unsigned int column) const;

		Vector2 GetDiagonal() const;
		void	SetDiagonal(const Vector2& in);

		inline Vector2 operator*(const Vector2& v) const;

		static Matrix2 Rotation(float degrees);

		void	Invert();
		Matrix2 Inverse() const;

		//Handy string output for the matrix. Can get a bit messy, but better than nothing!
		inline friend std::ostream& operator<<(std::ostream& o, const Matrix2& m);
	};

	//Handy string output for the matrix. Can get a bit messy, but better than nothing!
	inline std::ostream& operator<<(std::ostream& o, const Matrix2& m) {
		o << "Mat2(";
		o << "\t" << m.array[0][0] << "," << m.array[0][1] << "\n";
		o << "\t\t" << m.array[1][0] << "," << m.array[1][1] << "\n";
		return o;
	}
}