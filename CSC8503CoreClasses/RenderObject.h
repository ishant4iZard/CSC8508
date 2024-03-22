#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include <Buffer.h>
#include "MeshAnimation.h"


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
		
		enum class AnimationType : uint8_t
		{
			MALEGUARD_IDLE,
			MALEGUARD_GUNFIRE,
			MALEGUARD_STEPFORWARD,

			MAXGUARD_IDLE,
			MAXGUARD_GUNFIRE,
			MAXGUARD_WALK,

			MAX_ANM
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

			Buffer* GetGPUBuffer() const {
				return buffer;
			}

			void SetGPUBuffer(Buffer* b) {
				buffer = b;
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

			void SetShader(Shader* inShader) { this->shader = inShader; }
			void SetMesh(Mesh* inMesh) { this->mesh = inMesh; }
			void SetColour(const Vector4& c) {
				colour = c;
			}
			void SetTexture(const TextureType& inType, Texture* inTex)
			{
				if (inType == TextureType::MAX_TYPE) return;
				textureList[(uint8_t)inType] = inTex;
			}

			void SetTextureAnm(const std::string& texType, int index, Texture* inTex) {
				if (texType == "Diffuse") {
					anmObjDiffuseTextureList[index] = inTex;
				}
				if (texType == "Bump") {
					anmObjBumpTextureList[index] = inTex;
				}
			}

			Texture* GetTextureAnm(const std::string& texType, int index) {
				if (texType == "Diffuse") {
					return anmObjDiffuseTextureList[index];
				}
				if (texType == "Bump") {
					return anmObjBumpTextureList[index];
				}
				return nullptr;
			}

			Vector4 GetColour() const {
				return colour;
			}

			Vector2 GetTiling() const { return tiling; }
			void SetTiling(const Vector2& inTiling) { this->tiling = inTiling; }

			void SetAnimation(MeshAnimation* inAnim)
			{
				anim = inAnim;

				if (!anim) return;

				if (animDefault == nullptr) {
					animDefault = anim;
				}
				AnimFrame[objID] = 0.0f;

				skeleton.resize(anim->GetJointCount());
			}

			void UpdateAnimation(float dt)
			{
				static const float ANIMATION_SPEED = 500;

				if (!mesh || !anim) {
					return;
				}
				
#ifdef _WIN32
				animTime -= dt;

				if (animTime <= 0) {

					if (AnimFrame[objID] + 1 >= anim->GetFrameCount()) {
						SetAnimation(animDefault);
					}

					animTime += 1.0f / anim->GetFrameTime();
					AnimFrame[objID] = (++AnimFrame[objID]) % anim->GetFrameCount();
					currentAnimFrame = (++currentAnimFrame) % anim->GetFrameCount();

					//To do...
					//each submesh may have its own matrix
					std::vector<Matrix4>const& inverseBindPose = mesh->GetInverseBindPose();

					if (inverseBindPose.size() != anim->GetJointCount()) {
						//oh no
						//return;
					}

					const Matrix4* joints = anim->GetJointData(AnimFrame[objID]);

					for (int i = 0; i < skeleton.size(); ++i) {
						skeleton[i] = joints[i] * inverseBindPose[i];
					}
				}
#else
				animTime -= dt * ANIMATION_SPEED;

				if (animTime <= 0) {
					currentAnimFrame++;
					animTime += anim->GetFrameTime();
					currentAnimFrame = (currentAnimFrame++) % anim->GetFrameCount();

					std::vector<Matrix4>const& inverseBindPose = mesh->GetInverseBindPose();

					if (inverseBindPose.size() != anim->GetJointCount()) {
						//oh no
						return;
					}

					const Matrix4* joints = anim->GetJointData(currentAnimFrame);

					for (int i = 0; i < skeleton.size(); ++i) {
						skeleton[i] = joints[i] * inverseBindPose[i];
					}
				}
#endif
			}

			void ResetAnimation(int idleFrame) {
				currentAnimFrame = idleFrame;

				std::vector<Matrix4>const& inverseBindPose = mesh->GetInverseBindPose();

				if (inverseBindPose.size() != anim->GetJointCount()) {
					//oh no
					return;
				}

				const Matrix4* joints = anim->GetJointData(currentAnimFrame);

				for (int i = 0; i < skeleton.size(); ++i) {
					skeleton[i] = joints[i] * inverseBindPose[i];
				}
			}

			std::vector<Matrix4>& GetSkeleton() {
				return skeleton;
			}

			void SetID(int id) { objID = id; }

		protected:
			Buffer* buffer;
			Mesh*		mesh;
			Texture*	texture;
			Texture* textureList[(uint8_t)TextureType::MAX_TYPE];
			Shader*		shader;
			Transform*	transform;
			Vector4		colour;
			Vector2 tiling;
			const char* textureNameList[(uint8_t)TextureType::MAX_TYPE] = {
				"diffuseTex",
				"normalTex",
				"metallicTex",
				"roughnessTex",
				"ambiantOccTex" };

			MeshAnimation* anim = nullptr;
			MeshAnimation* animDefault = nullptr;

			std::vector<Matrix4> skeleton;
			float	animTime = 0.0f;
			int currentAnimFrame = 0;

			int AnimFrame[4] = {0};
			int objID = 0;

			Texture* anmObjDiffuseTextureList[4];
			Texture* anmObjBumpTextureList[4];
		};
	}
}
