#pragma once

using std::vector;

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Transform
		{
		public:
			Transform();
			~Transform();

			Transform& SetPosition(const Vector3& worldPos);
			Transform& SetScale(const Vector3& worldScale);
			Transform& SetOrientation(const Quaternion& newOr);
			Transform& RandomPosition(const Vector3& worldPos,bool openorclose);
			float randomFloat(float min, float max);

			Vector3 GetPosition() const {
				return position;
			}

			Vector3 GetScale() const {
				return scale;
			}

			Quaternion GetOrientation() const {
				return orientation;
			}

			Matrix4 GetMatrix() const {
				return matrix;
			}
			void UpdateMatrix();
		protected:
			Matrix4		matrix;

			Quaternion	orientation;
			Vector3		position;
			Vector3		scale;

			float minX=-50;
			float minY;
			float maxX=50;
			float maxY;
			float minZ=-50;
			float maxZ=50;
			const Vector3 UnitX = Vector3(1.0f, 0.0f, 0.0f);
			const Vector3 UnitZ = Vector3(0.0f, 0.0f, 1.0f);
		};
	}
}

