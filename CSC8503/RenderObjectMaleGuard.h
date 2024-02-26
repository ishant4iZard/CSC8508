#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

#include "RenderObject.h"

#include "MeshMaterial.h"
#include "MeshAnimation.h"

#include <vector>

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObjectMaleGuard : public RenderObject
		{
		public:
			RenderObjectMaleGuard(Transform* parentTransform, Mesh* mesh, Texture* tex, Shader* shader, MeshMaterial* material);
			~RenderObjectMaleGuard();

			void SetAnimation(MeshAnimation* animation) { maleGuardAnimation = animation; }

			void SetMaleGuardPosition(Vector3 inPos);
			Vector3 GetMaleGuardPosition();

			void SetMaleGuardScale(Vector3 inScale);
			Vector3 GetMaleGuardScale();

			void SetMaleGuardRotation(Vector4 inRotation);
			Vector4 GetMaleGuardRotation();

		protected:

			MeshMaterial* maleGuardMaterial;
			MeshAnimation* maleGuardAnimation;

			std::vector<unsigned int> diffuseTextures;
			std::vector<unsigned int> bumpTextures;

			Vector3 maleGuardPos;
			Vector3 maleGuardScale;
			Vector3 maleGuardRotation;
			float rotateDegrees;
		};
	}
}