#pragma once
#include "../CSC8503CoreClasses/GameWorld.h"

#include "GameTechRendererInterface.h"

#include "../PS5Core/AGCRenderer.h"

#include "../PS5Core/AGCBuffer.h"

#include "../Assets/Shaders/PSSL/Interop.h"				//Always include this before any PSSL headers
#include "../Assets/Shaders/PSSL/ShaderConstants.psslh"
#include "../Assets/Shaders/PSSL/TechObject.psslh"
#include "GameTechAGCModel.h"
#include "FrameData.h"

namespace NCL {
	namespace Rendering {
		class Mesh;
		class Texture;
		class Shader;
	}
	namespace PS5 {
		class AGCTexture;
		class AGCShader;
	}

	namespace CSC8503 {
		class RenderObject;

		class GameTechAGCRenderer :
			public NCL::PS5::AGCRenderer,
			public NCL::CSC8503::GameTechRendererInterface
		{
		public:
			GameTechAGCRenderer(GameWorld& world);
			~GameTechAGCRenderer();

			virtual Mesh* LoadMesh(const std::string& name)				override;
			virtual Texture* LoadTexture(const std::string& name)			override;
			virtual Shader* LoadShader(const std::string& vertex, const std::string& fragment)	override;

		protected:
			void RenderFrame()	override;
			void UpdateObjectList();
			void SetPbrTexture(ObjectState* outState, RenderObject* inRenderObj);

			NCL::PS5::AGCTexture* CreateFrameBufferTextureSlot(const std::string& name);

			vector<const RenderObject*> activeObjects;

			void WriteRenderPassConstants();
			void DrawObjects();
			void UpdateDebugData();

			void RenderDebugLines();
			void RenderDebugText();

			void ShadowmapPass();
			void SkyboxPass();
			void MainRenderPass();

			void DisplayRenderPass();

			void GPUSkinningPass();

			GameWorld& gameWorld;

			struct SkinningJob {
				RenderObject* object;
				uint32_t outputIndex;
			};

			FrameData* allFrames;
			FrameData* currentFrame;
			/// <summary>
			/// Frame index to implement double buffer pattern.
			/// </summary>
			int currentFrameIndex;

			NCL::PS5::AGCMesh* quadMesh;

			sce::Agc::Core::Texture* bindlessTextures;
			sce::Agc::Core::Buffer* bindlessBuffers;
			uint32_t bufferCount;

			sce::Agc::Core::Buffer textureBuffer;
			std::map<std::string, NCL::PS5::AGCTexture*> textureMap;

			sce::Agc::Core::Buffer arrayBuffer;

			GameTechAGCModel* rendererModel;
			sce::Agc::CxDepthRenderTarget		shadowTarget;
			NCL::PS5::AGCTexture* shadowMap; //ptr into bindless array
			sce::Agc::Core::Sampler				shadowSampler;

			sce::Agc::CxRenderTarget			screenTarget;
			NCL::PS5::AGCTexture* screenTex; //ptr into bindless array

			std::vector<SkinningJob> frameJobs;
		};
	}
}