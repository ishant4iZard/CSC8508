#pragma once
#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"

#include "GameWorld.h"

#include "MeshMaterial.h"
#include "MeshAnimation.h"
#include "Assets.h"
#include "GameAnimation.h"

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

		class GameTechRenderer : public OGLRenderer	{
		public:
			GameTechRenderer(GameWorld& world);
			~GameTechRenderer();

			Mesh*		LoadMesh(const std::string& name);
			Texture*	LoadTexture(const std::string& name);
			Shader*		LoadShader(const std::string& vertex, const std::string& fragment);

			void Update(float dt) override;

		protected:
			void NewRenderLines();
			void NewRenderText();

			void RenderFrame()	override;

			OGLShader*		defaultShader;

			GameWorld&	gameWorld;

			void BuildObjectList();
			void SortObjectList();
			void RenderShadowMap();
			void RenderCamera(); 
			void RenderSkybox();

			void LoadSkybox();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			vector<const RenderObject*> activeObjects;

			OGLShader*  debugShader;
			OGLShader*  skyboxShader;
			OGLMesh*	skyboxMesh;
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

			//Skeletal Animation
			void LoadAnimationAssets();
			void LoadTextureToMesh();
			void RenderAnimation(Vector3 inPos, Vector3 inScale, Vector4 inRotation, int animatedObjectID, string name);
			void LoadCurrentAnimationAssets(OGLShader* currentShader, MeshMaterial* currentMaterial, MeshAnimation* currentAnimation, int animatedObjectID);

			void Matrix4ToIdentity(Matrix4* mat4);

			OGLShader* anmShader;

			Mesh* maleGuardMesh = nullptr;
			Mesh* maxGuardMesh = nullptr;
			
			MeshMaterial* maleGuardMaterial;
			MeshMaterial* maxGuardMaterial;


			vector<GLuint> maleGuardMatDiffuseTextures;
			vector<GLuint> maleGuardMatBumpTextures;
			vector<GLuint> femaleGuardMatDiffuseTextures;
			vector<GLuint> femaleGuardMatBumpTextures;
			vector<GLuint> maxGuardMatDiffuseTextures;
			vector<GLuint> maxGuardMatBumpTextures;

			MeshAnimation* maleGuardAnimationGunfire1;//Gunfire1.anm
			MeshAnimation* maleGuardAnimationHappy;//Happy.anm
			MeshAnimation* activeAnimation[4];

			int currentFrame[4] = { 0 };
			float frameTime[4] = { 0.0f };

			GLuint currentShaderID;
			bool hasLoadedTextureToSubmesh = false;

			OGLTexture* albedo = nullptr;
			OGLTexture* normal = nullptr;
			OGLTexture* metallic = nullptr;
			OGLTexture* roughness = nullptr;
			OGLTexture* ao = nullptr;

			vector<const GameObject*> activeAnimatedObjects;
			GameAnimation* animatedObjects;

			void RenderAnimatedObject();
			void RenderMaleGuard(GameObject* maleGuard);
			void RenderMaxGuard(GameObject* maxGuard);
			//Skeletal Animation


#pragma region UI
			UIBase* ui;
#pragma endregion
		};
	}
}

