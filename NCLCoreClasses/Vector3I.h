/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

namespace NCL::Maths {
	class Vector2i;
	class Vector4i;

	class Vector3i {
	public:
		int x;
		int y;
		int z;
	public:
		constexpr Vector3i(void) : x(0), y(0), z(0) {}

		constexpr Vector3i(int xVal, int yVal, int zVal) : x(xVal), y(yVal), z(zVal) {}

		Vector3i(const Vector2i& v2, int z = 0);
		Vector3i(const Vector4i& v4);

		~Vector3i(void) = default;

		constexpr int		GetMinElement() const {
			int v = x;
			v = std::min(v, y);
			v = std::min(v, z);
			return v;
		}

		constexpr int		GetMaxElement() const {
			int v = x;
			v = std::max(v, y);
			v = std::max(v, z);
			return v;
		}

		int		GetAbsMaxElement() const {
			int v = std::abs(x);
			v = std::max(v, std::abs(y));
			v = std::max(v, std::abs(z));
			return v;
		}

		static constexpr Vector3i Clamp(const Vector3i& input, const Vector3i& mins, const Vector3i& maxs);

		inline Vector3i  operator+(const Vector3i& a) const {
			return Vector3i(x + a.x, y + a.y, z + a.z);
		}

		inline Vector3i  operator-(const Vector3i& a) const {
			return Vector3i(x - a.x, y - a.y, z - a.z);
		}

		inline Vector3i  operator-() const {
			return Vector3i(-x, -y, -z);
		}

		inline Vector3i  operator*(int a)	const {
			return Vector3i(x * a, y * a, z * a);
		}

		inline Vector3i  operator*(const Vector3i& a) const {
			return Vector3i(x * a.x, y * a.y, z * a.z);
		}

		inline Vector3i  operator/(const Vector3i& a) const {
			return Vector3i(x / a.x, y / a.y, z / a.z);
		};

		inline Vector3i  operator/(int v) const {
			return Vector3i(x / v, y / v, z / v);
		};

		inline constexpr void operator+=(const Vector3i& a) {
			x += a.x;
			y += a.y;
			z += a.z;
		}

		inline void operator-=(const Vector3i& a) {
			x -= a.x;
			y -= a.y;
			z -= a.z;
		}

		inline void operator*=(const Vector3i& a) {
			x *= a.x;
			y *= a.y;
			z *= a.z;
		}

		inline void operator/=(const Vector3i& a) {
			x /= a.x;
			y /= a.y;
			z /= a.z;
		}

		inline void operator*=(int f) {
			x *= f;
			y *= f;
			z *= f;
		}

		inline void operator/=(int f) {
			x /= f;
			y /= f;
			z /= f;
		}

		inline int operator[](int i) const {
			return ((int*)this)[i];
		}

		inline int& operator[](int i) {
			return ((int*)this)[i];
		}

		inline bool	operator==(const Vector3i& A)const { return (A.x == x && A.y == y && A.z == z) ? true : false; };
		inline bool	operator!=(const Vector3i& A)const { return (A.x == x && A.y == y && A.z == z) ? false : true; };

		inline friend std::ostream& operator<<(std::ostream& o, const Vector3i& v) {
			o << "Vector3i(" << v.x << "," << v.y << "," << v.z << ")\n";
			return o;
		}
	};
}