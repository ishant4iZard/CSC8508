#pragma once
#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"
#include "Event.h"
#include "PowerUp.h"
#include "GameWorld.h"
#include <Vector3.h>
#include <Vector4.h>

#include "../CSC8503/UIBase.h"
#ifdef _WIN32
#include "../CSC8503/UIWindows.h"
#endif

namespace NCL {
	//class Maths::Vector3;
	//class Maths::Vector4;

	using namespace Maths;

	namespace CSC8503 {
		class RenderObject;
		class DirectionalLight;
		class OglHdrFbo;
		class OglPostProcessingFbo;

		class GameTechRenderer : public OGLRenderer, public EventListener {
		public:
			GameTechRenderer(GameWorld& world);
			void CreateScreenQuadMesh();
			~GameTechRenderer();

			Mesh*		LoadMesh(const std::string& name);
			Texture*	LoadTexture(const std::string& name);
			Shader*		LoadShader(const std::string& vertex, const std::string& fragment);

			void ReceiveEvent(EventType eventType) override;
			OGLShader* GetShader() const { return particleShader; }

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
			OGLShader* particleShader;

			//shadow mapping things
			OGLShader*	shadowShader;
			OGLShader*	testShader;
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
			Texture* windTexture;
			Texture* sandTexture;

			//frost fbo
			OglPostProcessingFbo* frostFbo;

			powerUpType currentActivePowerup = none;
			float timeOfPowerupActivation = 0;
			const float POST_PROCESSING_DURATION = 5; // seconds

			UIBase* ui;

			DirectionalLight* directionalLight;

			OglHdrFbo* pbrFbo;
			OglPostProcessingFbo* toneMappingFbo;

			float timeOfPortalCollision = 0;
			bool wasPortalCollided;
			const float PORTAL_BLINK_TIME = 0.5;
		};
	}
}
