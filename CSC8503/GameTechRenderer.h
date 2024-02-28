#pragma once
#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"

#include "GameWorld.h"

#include "../CSC8503/UIBase.h"
#ifdef _WIN32
#include "../CSC8503/UIWindows.h"
#else //_ORBIS
#include "../CSC8503/UIPlaystation.h"
#endif

namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class RenderObject;
		class DirectionalLight;
		class OglHdrFbo;
		class OglPostProcessingFbo;

		class GameTechRenderer : public OGLRenderer	{
		public:
			GameTechRenderer(GameWorld& world);
			void CreateScreenQuadMesh();
			~GameTechRenderer();

			Mesh*		LoadMesh(const std::string& name);
			Texture*	LoadTexture(const std::string& name);
			Shader*		LoadShader(const std::string& vertex, const std::string& fragment);

		protected:
			void NewRenderLines();
			void NewRenderText();

			void RenderFrame()	override;

			OGLShader*		defaultShader;

			GameWorld&	gameWorld;

			void BuildObjectList();
			void SortObjectList();
			void RenderShadowMap();
			void UpdatePBRUniforms(const NCL::CSC8503::RenderObject* const& i);
			void UpdateGlobalLightUniform(const OGLShader* inShader);
			void RenderCamera();
			void RenderInstancedRenderObject();
			void RenderSkybox();
			void ApplyFrostingPostProcessing();
			void ApplyToneMapping();
			void RenderProcessedScene();
			void LoadSkybox();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			vector<const RenderObject*> activeObjects;
			vector<const RenderObject*> instancedRenderObjectList;

			OGLShader*  debugShader;
			OGLShader*  skyboxShader;
			OGLShader* pbrShader;
			OGLShader* toneMapperShader;
			OGLShader* gammaCorrectionShader;

			OGLShader* frostPostProcessing;
			OGLMesh*	skyboxMesh;
			OGLMesh* screenQuad;
			GLuint		skyboxTex;

			//shadow mapping things
			OGLShader*	shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;
			Matrix4     shadowMatrix;

			Vector4		lightColour;
			float		lightRadius;
			Vector3		lightPosition;

			//Debug data storage things
			vector<Vector3> debugLineData;

			vector<Vector3> debugTextPos;
			vector<Vector4> debugTextColours;
			vector<Vector2> debugTextUVs;

			GLuint lineVAO;
			GLuint lineVertVBO;
			size_t lineCount;

			GLuint textVAO;
			GLuint textVertVBO;
			GLuint textColourVBO;
			GLuint textTexVBO;
			size_t textCount;

			//Texture
			Texture* frostTexture;

#pragma region UI
			UIBase* ui;
#pragma endregion

#pragma region Lights
			DirectionalLight* directionalLight;
#pragma endregion

#pragma region FrameBuffers
			OglHdrFbo* pbrFbo;
			OglPostProcessingFbo* toneMappingFbo;
#pragma endregion

		};
	}
}
