/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

namespace NCL::Maths {
	class Vector2;
	class Vector4;

	class Vector3 {
	public:
		float x;
		float y;
		float z;
	public:
		constexpr Vector3(void) : x(0.0f), y(0.0f), z(0.0f) {}

		constexpr Vector3(float xVal, float yVal, float zVal) : x(xVal), y(yVal), z(zVal) {}

		Vector3(const Vector2& v2, float z = 0.0f);
		Vector3(const Vector4& v4);

		~Vector3(void) = default;

		Vector3 Normalised() const {
			Vector3 temp(*this);
			temp.Normalise();
			return temp;
		}

		void abs() {
			std::abs(x);
			std::abs(y);
			std::abs(z);

		}

		void			Normalise() {
			float length = Length();

			if (length != 0.0f) {
				length = 1.0f / length;
				x = x * length;
				y = y * length;
				z = z * length;
			}
		}

		inline  float	Length() const {
			return std::sqrt(LengthSquared());
		}

		inline constexpr float	LengthSquared() const {
			return ((x*x) + (y*y) + (z*z));
		}

		constexpr float		GetMinElement() const {
			float v = x;
			v = std::min(v, y);
			v = std::min(v, z);
			return v;
		}

		constexpr float		GetMaxElement() const {
			float v = x;
			v = std::max(v, y);
			v = std::max(v, z);
			return v;
		}

		float		GetAbsMaxElement() const {
			float v = std::abs(x);
			v = std::max(v, std::abs(y));
			v = std::max(v, std::abs(z));
			return v;
		}

		static  Vector3 Clamp(const Vector3& input, const Vector3& mins, const Vector3& maxs);

		static constexpr float	Dot(const Vector3& a, const Vector3& b) {
			return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
		}

		static Vector3	Cross(const Vector3& a, const Vector3& b) {
			return Vector3((a.y*b.z) - (a.z*b.y), (a.z*b.x) - (a.x*b.z), (a.x*b.y) - (a.y*b.x));
		}

		inline Vector3  operator+(const Vector3& a) const {
			return Vector3(x + a.x, y + a.y, z + a.z);
		}

		inline Vector3  operator-(const Vector3& a) const {
			return Vector3(x - a.x, y - a.y, z - a.z);
		}

		inline Vector3  operator-() const {
			return Vector3(-x, -y, -z);
		}

		inline Vector3  operator*(float a)	const {
			return Vector3(x * a, y * a, z * a);
		}

		inline Vector3  operator*(const Vector3& a) const {
			return Vector3(x * a.x, y * a.y, z * a.z);
		}

		inline Vector3  operator/(const Vector3& a) const {
			return Vector3(x / a.x, y / a.y, z / a.z);
		};

		inline Vector3  operator/(float v) const {
			return Vector3(x / v, y / v, z / v);
		};

		inline constexpr void operator+=(const Vector3& a) {
			x += a.x;
			y += a.y;
			z += a.z;
		}

		inline void operator-=(const Vector3& a) {
			x -= a.x;
			y -= a.y;
			z -= a.z;
		}

		inline void operator*=(const Vector3& a) {
			x *= a.x;
			y *= a.y;
			z *= a.z;
		}

		inline void operator/=(const Vector3& a) {
			x /= a.x;
			y /= a.y;
			z /= a.z;
		}

		inline void operator*=(float f) {
			x *= f;
			y *= f;
			z *= f;
		}

		inline void operator/=(float f) {
			x /= f;
			y /= f;
			z /= f;
		}

		inline float operator[](int i) const {
			return ((float*)this)[i];
		}

		inline float& operator[](int i) {
			return ((float*)this)[i];
		}

		inline bool	operator==(const Vector3& vec)const { return (vec.x == x && vec.y == y && vec.z == z) ? true : false; };
		inline bool	operator!=(const Vector3& vec)const { return (vec.x == x && vec.y == y && vec.z == z) ? false : true; };

		inline friend std::ostream& operator<<(std::ostream& o, const Vector3& vec) {
			o << "Vector3(" << vec.x << "," << vec.y << "," << vec.z << ")\n";
			return o;
		}

		static Vector3 MoveTowards(Vector3 current, Vector3 target, float maxDistanceDelta)
		{
			// avoid vector ops because current scripting backends are terrible at inlining
			float toVector_x = target.x - current.x;
			float toVector_y = target.y - current.y;
			float toVector_z = target.z - current.z;

			float sqdist = toVector_x * toVector_x + toVector_y * toVector_y + toVector_z * toVector_z;

			if (sqdist == 0 || (maxDistanceDelta >= 0 && sqdist <= maxDistanceDelta * maxDistanceDelta))
				return target;
			float dist = (float)sqrt(sqdist);

			return Vector3(current.x + toVector_x / dist * maxDistanceDelta,
				current.y + toVector_y / dist * maxDistanceDelta,
				current.z + toVector_z / dist * maxDistanceDelta);
		}
	};
}