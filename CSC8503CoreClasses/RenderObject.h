#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		enum class TextureType : uint8_t
		{
			ALBEDO = 0,
			NORMAL,
			METAL,
			ROUGHNESS,
			AO,
			MAX_TYPE
		};

		class RenderObject
		{
		public:
			RenderObject(Transform* parentTransform, Mesh* mesh, Texture* tex, Shader* shader);
			~RenderObject();

			void SetDefaultTexture(Texture* t) {
				texture = t;
			}

			Texture* GetDefaultTexture() const {
				return texture;
			}

			Texture* GetTexture(const TextureType& inType) const
			{
				if(inType != TextureType::MAX_TYPE)
					return textureList[(uint8_t)inType];
				return nullptr;
			}

			const char* GetTextureName(const TextureType& inType) const
			{
				if (inType != TextureType::MAX_TYPE)
					return textureNameList[(uint8_t)inType];
				return "";
			}

			Mesh*	GetMesh() const {
				return mesh;
			}

			Transform*		GetTransform() const {
				return transform;
			}

			Shader*		GetShader() const {
				return shader;
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}
			void SetTexture(const TextureType& inType, Texture* inTex)
			{
				if (inType == TextureType::MAX_TYPE) return;
				textureList[(uint8_t)inType] = inTex;
			}

			Vector4 GetColour() const {
				return colour;
			}

		protected:
			Mesh*		mesh;
			Texture*	texture;
			Texture* textureList[(uint8_t)TextureType::MAX_TYPE];
			Shader*		shader;
			Transform*	transform;
			Vector4		colour;
			const char* textureNameList[(uint8_t)TextureType::MAX_TYPE] = {
				"diffuseTex",
				"normalTex",
				"metallicTex",
				"roughnessTex",
				"ambiantOccTex" };
		};
	}
}
