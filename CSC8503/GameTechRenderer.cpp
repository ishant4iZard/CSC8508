#include "GameTechRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "RenderObjectMaleGuard.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"

#include "MaleGuard.h"
#include "MaxGuard.h"
#include "DirectionalLight.h"
#include "OglHdrFbo.h"
#include "OglPostProcessingFbo.h"
#include "NetworkPlayer.h"

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;
#define SHADOWSIZE 4096

Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f));

GameTechRenderer::GameTechRenderer(GameWorld& world) : OGLRenderer(*Window::GetWindow()), gameWorld(world)	{
	glEnable(GL_DEPTH_TEST);
	debugShader  = new OGLShader("debug.vert", "debug.frag");
	shadowShader = new OGLShader("shadow.vert", "shadow.frag");

	toneMapperShader = new OGLShader("basic.vert", "ReinhardTonemap.frag");
	pbrShader = new OGLShader("pbr.vert", "pbr.frag");
	gammaCorrectionShader = new OGLShader("basic.vert", "gammaCorrection.frag");

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			     SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.5f,0.5f,0.5f, 1);

	//Set up the light properties
	lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	lightRadius = 1000.0f;
	lightPosition = Vector3(-200.0f, 60.0f, -200.0f);

	//Setup directional light
	directionalLight = new DirectionalLight(
		Vector3(-0.2f, -220.0f, -100.0f),
		0.02f,
		Vector4(247, 149, 64, 1.0f));
	//Skybox!
	skyboxShader = new OGLShader("skybox.vert", "skybox.frag");
	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	CreateScreenQuadMesh();

	LoadSkybox();

	glGenVertexArrays(1, &lineVAO);
	glGenVertexArrays(1, &textVAO);

	glGenBuffers(1, &lineVertVBO);
	glGenBuffers(1, &textVertVBO);
	glGenBuffers(1, &textColourVBO);
	glGenBuffers(1, &textTexVBO);

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadTexture("PressStart2P.png"));

	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);

	for (int i = 0; i < 4; i++) {
		activeAnimation[i] = nullptr;
		animationDefault[i] = nullptr;
		animationStateCounter[i] = 0;
	}
	pbrFbo = new OglHdrFbo(windowSize.x, windowSize.y);
	toneMappingFbo = new OglPostProcessingFbo(windowSize.x, windowSize.y);

#ifdef _WIN32
	ui = UIWindows::GetInstance();
#else //_ORBIS
	ui = UIPlaystation::GetInstance();
#endif

	EventEmitter::RegisterForEvent(PROJECTILE_PORTAL_COLLISION, this);
	timeOfPortalCollision = 0;
	wasPortalCollided = false;
}

void GameTechRenderer::CreateScreenQuadMesh()
{
	screenQuad = new OGLMesh();
	screenQuad->SetPrimitiveType(GeometryPrimitive::TriangleStrip);
	screenQuad->SetVertexPositions({ Vector3(-1, 1,0), Vector3(-1,-1,0) , Vector3(1,1,0) , Vector3(1,-1,0) });
	screenQuad->SetVertexTextureCoords({ Vector2(0.0f, 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 0.0f) });
	screenQuad->UploadToGPU();
}

GameTechRenderer::~GameTechRenderer()	{
	EventEmitter::RemoveListner(this);
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	SAFE_DELETE(directionalLight);
	SAFE_DELETE(pbrFbo);
	SAFE_DELETE(toneMappingFbo);
	SAFE_DELETE(toneMapperShader);
}

void GameTechRenderer::LoadSkybox() {
	std::string filenames[6] = {
		"/SpaceCubeMap/3.png",
		"/SpaceCubeMap/0.png",
		"/SpaceCubeMap/2.png",
		"/SpaceCubeMap/5.png",
		"/SpaceCubeMap/1.png",
		"/SpaceCubeMap/4.png"
	};

	int width[6]	= { 0 };
	int height[6]	= { 0 };
	int channels[6] = { 0 };
	int flags[6]	= { 0 };

	vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) {
		TextureLoader::LoadTexture(filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return;
		}
	}
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::RenderFrame() {
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1);
	BuildObjectList();
	SortObjectList();
	//RenderShadowMap();
	//RenderSkybox();
	RenderCamera();
	ApplyToneMapping();
	ApplyFrostingPostProcessing();
	RenderProcessedScene();
	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	NewRenderLines();
	NewRenderText();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	UIWindows::GetInstance()->RenderUI();
}

void GameTechRenderer::BuildObjectList() {
	activeObjects.clear();
	activeAnimatedObjects.clear();
	instancedRenderObjectList.clear();
	//gameWorld.gameObjectsMutex.lock();
	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {

				const RenderObject* g = o->GetRenderObject();
				OGLShader* currentShader;


				//find MaleGuard and load assets, especially the animation in current frame
				if (o->GetName() == "MaleGuard") {
					currentShader = (OGLShader*)o->GetRenderObject()->GetShader();
					currentShaderID = currentShader->GetProgramID();
					NetworkPlayer* maleGuard = dynamic_cast<NetworkPlayer*>(o);

					LoadCurrentAnimationAssets(currentShader, nullptr, maleGuard->GetAnimation(), maleGuard->GetAnimatedObjectID(), maleGuard->GetAnimationStateCounter());

					activeAnimatedObjects.emplace_back(maleGuard);
				}
				//if (o->GetName() == "MaxGuard") {
				//	currentShader = (OGLShader*)o->GetRenderObject()->GetShader();
				//	currentShaderID = currentShader->GetProgramID();
				//	//MaxGuard* maxGuard = (MaxGuard*)o;
				//	MaxGuard* maxGuard = dynamic_cast<MaxGuard*>(o);
				//	
				//	LoadCurrentAnimationAssets(currentShader, maxGuard->GetMaterial(), maxGuard->GetAnimation(), maxGuard->GetAnimatedObjectID(),);

				//	activeAnimatedObjects.emplace_back(maxGuard);
				//}

				if (g) {
					if (g->GetMesh()->GetInstanceCount() > 0)
						instancedRenderObjectList.emplace_back(g);
					else
						activeObjects.emplace_back(g);
				}
			}
		}
	);
	//gameWorld.gameObjectsMutex.unlock();
}

void GameTechRenderer::SortObjectList() {

}

void GameTechRenderer::RenderShadowMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	BindShader(*shadowShader);
	int mvpLocation = glGetUniformLocation(shadowShader->GetProgramID(), "mvpMatrix");

	Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(lightPosition, Vector3(0, 0, 0), Vector3(0,1,0));
	Matrix4 shadowProjMatrix = Matrix4::Perspective(100.0f, 500.0f, 1, 45.0f);

	Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

	shadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

	for (const auto& tempRenderObj : activeObjects) {
		Matrix4 modelMatrix = (*tempRenderObj).GetTransform()->GetMatrix();
		Matrix4 mvpMatrix	= mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);
		BindMesh((OGLMesh&)*(*tempRenderObj).GetMesh());
		size_t layerCount = (*tempRenderObj).GetMesh()->GetSubMeshCount();
		for (size_t i = 0; i < layerCount; ++i) {
			DrawBoundMesh((uint32_t)i);
		}
	}

	glViewport(0, 0, (float)windowSize.x, (float)windowSize.y);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
}

void GameTechRenderer::RenderSkybox() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	BindShader(*skyboxShader);

	int projLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix");
	int texLocation  = glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

	glUniform1i(texLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	BindMesh(*skyboxMesh);
	DrawBoundMesh();

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void NCL::CSC8503::GameTechRenderer::ApplyFrostingPostProcessing()
{
}

void GameTechRenderer::ApplyToneMapping()
{
	glBindFramebuffer(GL_FRAMEBUFFER, toneMappingFbo->GetFbo());
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	BindShader(*toneMapperShader);

	Matrix4 identityMatrix;
	identityMatrix.ToIdentity();

	int modelLocation = glGetUniformLocation(toneMapperShader->GetProgramID(), "modelMatrix");
	int projLocation = glGetUniformLocation(toneMapperShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(toneMapperShader->GetProgramID(), "viewMatrix");
	
	glUniformMatrix4fv(modelLocation, 1, false, (float*)&identityMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&identityMatrix);
	glUniformMatrix4fv(projLocation, 1, false, (float*)&identityMatrix);

	glUniform1i(glGetUniformLocation(toneMapperShader->GetProgramID(),
		"diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pbrFbo->GetColorBuffer());

	BindMesh(*screenQuad);
	DrawBoundMesh();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::RenderProcessedScene()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	glClear(GL_COLOR_BUFFER_BIT);

	Matrix4 identityMatrix;
	identityMatrix.ToIdentity();

	BindShader(*gammaCorrectionShader);

	int modelLocation = glGetUniformLocation(gammaCorrectionShader->GetProgramID(), "modelMatrix");
	int projLocation = glGetUniformLocation(gammaCorrectionShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(gammaCorrectionShader->GetProgramID(), "viewMatrix");

	glUniformMatrix4fv(modelLocation, 1, false, (float*)&identityMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&identityMatrix);
	glUniformMatrix4fv(projLocation, 1, false, (float*)&identityMatrix);

	glUniform1i(glGetUniformLocation(toneMapperShader->GetProgramID(),
		"diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, toneMappingFbo->GetColorBuffer());

	BindMesh(*screenQuad);
	DrawBoundMesh();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
}

void GameTechRenderer::UpdatePBRUniforms(const NCL::CSC8503::RenderObject* const& inRenderObj)
{
	for (uint8_t tempType = 0; tempType < (uint8_t)TextureType::MAX_TYPE; tempType++)
	{
		if ((*inRenderObj).GetTexture(static_cast<TextureType>(tempType)) == nullptr) continue;

		BindTextureToShader(
			*(OGLTexture*)(*inRenderObj).GetTexture(static_cast<TextureType>(tempType)),
			(*inRenderObj).GetTextureName(static_cast<TextureType>(tempType)), (int)tempType + 1); //maintex is 0 and shadow tex is 1,hence we are moving PBR texture data ahead by 2 that makes is 2,3,4,....
	}
}

void GameTechRenderer::UpdateGlobalLightUniform(const OGLShader* inShader)
{
	GLint lightDirLocation = glGetUniformLocation(inShader->GetProgramID(), "globalLightDirection");
	GLint globalLightColorLocation = glGetUniformLocation(inShader->GetProgramID(), "globalLightColor");
	GLint globalIntensityLocation = glGetUniformLocation(inShader->GetProgramID(), "globalIntensity");

	glUniform3fv(lightDirLocation, 1, (float*)(&(directionalLight->GetDirection())));
	glUniform4fv(globalLightColorLocation, 1, (float*)(&(directionalLight->GetColor())));
	glUniform1f(globalIntensityLocation, directionalLight->GetIntensity());
}

void GameTechRenderer::RenderCamera() {
	glBindFramebuffer(GL_FRAMEBUFFER, pbrFbo->GetFbo());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	RenderSkybox();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	OGLShader* activeShader = nullptr;
	int projLocation	= 0;
	int viewLocation	= 0;
	int modelLocation	= 0;
	int tilingLocation = 0;
	int colourLocation  = 0;
	int hasVColLocation = 0;
	int hasTexLocation  = 0;
	int shadowLocation  = 0;

	int lightPosLocation	= 0;
	int lightColourLocation = 0;
	int lightRadiusLocation = 0;

	int cameraLocation = 0;

	//TODO - PUT IN FUNCTION
	//glActiveTexture(GL_TEXTURE0 + 1);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (const auto&tempRenderObj : activeObjects) {
		OGLShader* shader = (OGLShader*)(*tempRenderObj).GetShader();
		BindShader(*shader);

		//find maleGuard's shader
		//need to be rendered in animatedObject
		if (shader->GetProgramID() == currentShaderID) {
			continue;
		}


		// if ((*i).GetDefaultTexture()) {
		// 	BindTextureToShader(*(OGLTexture*)(*i).GetDefaultTexture(), "mainTex", 0);
		if ((*tempRenderObj).GetDefaultTexture()) {
			BindTextureToShader(*(OGLTexture*)(*tempRenderObj).GetDefaultTexture(), "mainTex", 0);
		}

		UpdatePBRUniforms(tempRenderObj);

		if (activeShader != shader) {
			projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
			viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
			modelLocation = glGetUniformLocation(shader->GetProgramID(), "modelMatrix");
			tilingLocation = glGetUniformLocation(shader->GetProgramID(), "tiling");
			shadowLocation = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
			colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
			hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
			hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");

			lightPosLocation = glGetUniformLocation(shader->GetProgramID(), "lightPos");
			lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
			lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");

			GLint timeLocation = glGetUniformLocation(shader->GetProgramID(), "time");

			GLint blackholeTexLocation = glGetUniformLocation(shader->GetProgramID(), "blackholeTex");
			GLint wasPortalCollidedLoc = glGetUniformLocation(shader->GetProgramID(), "wasPortalCollided");

			glUniform1f(timeLocation, time);

			if (timeOfPortalCollision + PORTAL_BLINK_TIME > time)
				wasPortalCollided = true;
			else
				wasPortalCollided = false;

			glUniform1i(wasPortalCollidedLoc, (wasPortalCollided ? 100 : -100));

			UpdateGlobalLightUniform(shader);

			cameraLocation = glGetUniformLocation(shader->GetProgramID(), "cameraPos");

			Vector3 camPos = gameWorld.GetMainCamera().GetPosition();
			glUniform3fv(cameraLocation, 1, &camPos.x);

			glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
			glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

			glUniform3fv(lightPosLocation	, 1, (float*)&lightPosition);
			glUniform4fv(lightColourLocation, 1, (float*)&lightColour);
			glUniform1f(lightRadiusLocation , lightRadius);

			//int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			//glUniform1i(shadowTexLocation, 1);

			activeShader = shader;
		}

		Matrix4 modelMatrix = (*tempRenderObj).GetTransform()->GetMatrix();
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);			
		
		const Vector2 tempTiling = (*tempRenderObj).GetTiling();
		glUniform2f(tilingLocation, tempTiling.x, tempTiling.y);

		Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		Vector4 colour = tempRenderObj->GetColour();
		glUniform4fv(colourLocation, 1, &colour.x);

		glUniform1i(hasVColLocation, !(*tempRenderObj).GetMesh()->GetColourData().empty());

		glUniform1i(hasTexLocation, (OGLTexture*)(*tempRenderObj).GetDefaultTexture() ? 1:0);

		BindMesh((OGLMesh&)*(*tempRenderObj).GetMesh());
		size_t layerCount = (*tempRenderObj).GetMesh()->GetSubMeshCount();
		for (size_t i = 0; i < layerCount; ++i) {
			DrawBoundMesh((uint32_t)i , (*tempRenderObj).GetMesh()->GetInstanceCount());
		}
	}

	RenderInstancedRenderObject();
	RenderAnimatedObject();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void NCL::CSC8503::GameTechRenderer::RenderInstancedRenderObject()
{
	OGLShader* activeShader = nullptr;

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	int projLocation = 0;
	int viewLocation = 0;
	int instancedmodelMatricesListLocation = 0;
	int tilingLocation = 0;
	int colourLocation = 0;
	int hasVColLocation = 0;
	int hasTexLocation = 0;
	int shadowLocation = 0;

	int lightPosLocation = 0;
	int lightColourLocation = 0;
	int lightRadiusLocation = 0;

	int cameraLocation = 0;
	const Mesh* previousRenderObject = nullptr;

	for (const auto& tempInstancedRenderObject : instancedRenderObjectList)
	{
		if (previousRenderObject == tempInstancedRenderObject->GetMesh()) continue;
		previousRenderObject = tempInstancedRenderObject->GetMesh();

		OGLShader* shader = (OGLShader*)(*tempInstancedRenderObject).GetShader();
		BindShader(*shader);
		if ((*tempInstancedRenderObject).GetDefaultTexture()) {
			BindTextureToShader(*(OGLTexture*)(*tempInstancedRenderObject).GetDefaultTexture(), "mainTex", 0);
		}

		UpdatePBRUniforms(tempInstancedRenderObject);

		if (activeShader != shader) {
			projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
			viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
			instancedmodelMatricesListLocation = glGetUniformLocation(shader->GetProgramID(), "instanceMatrix");
			tilingLocation = glGetUniformLocation(shader->GetProgramID(), "tiling");
			shadowLocation = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
			colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
			hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
			hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");

			lightPosLocation = glGetUniformLocation(shader->GetProgramID(), "lightPos");
			lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
			lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");

			UpdateGlobalLightUniform(shader);

			cameraLocation = glGetUniformLocation(shader->GetProgramID(), "cameraPos");

			Vector3 camPos = gameWorld.GetMainCamera().GetPosition();
			glUniform3fv(cameraLocation, 1, &camPos.x);

			glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
			glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

			glUniform3fv(lightPosLocation, 1, (float*)&lightPosition);
			glUniform4fv(lightColourLocation, 1, (float*)&lightColour);
			glUniform1f(lightRadiusLocation, lightRadius);

			//int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			//glUniform1i(shadowTexLocation, 1);

			activeShader = shader;
		}

		auto modelMatrixList = (*tempInstancedRenderObject).GetMesh()->GetInstanceModelMatricesData();
		glUniformMatrix4fv(instancedmodelMatricesListLocation, 1, false, (float*)&modelMatrixList);

		const Vector2 tempTiling = (*tempInstancedRenderObject).GetTiling();
		glUniform2f(tilingLocation, tempTiling.x, tempTiling.y);

		Matrix4 fullShadowMat; // TODO calculate it in vertex shader = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		Vector4 colour = tempInstancedRenderObject->GetColour();
		glUniform4fv(colourLocation, 1, &colour.x);

		glUniform1i(hasVColLocation, !(*tempInstancedRenderObject).GetMesh()->GetColourData().empty());

		glUniform1i(hasTexLocation, (OGLTexture*)(*tempInstancedRenderObject).GetDefaultTexture() ? 1 : 0);

		BindMesh((OGLMesh&)*(*tempInstancedRenderObject).GetMesh());
		size_t layerCount = (*tempInstancedRenderObject).GetMesh()->GetSubMeshCount();
		for (size_t i = 0; i < layerCount; ++i) {
			DrawBoundMesh((uint32_t)i, (*tempInstancedRenderObject).GetMesh()->GetInstanceCount());
		}
	}
}

Mesh* GameTechRenderer::LoadMesh(const std::string& name) {
	OGLMesh* mesh = new OGLMesh();
	MshLoader::LoadMesh(name, *mesh);
	mesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	mesh->UploadToGPU();
	return mesh;
}

void GameTechRenderer::RenderAnimatedObject() {
	for (const auto& i : activeAnimatedObjects) {
		if (i->GetName() == "MaleGuard") {
			MaleGuard* maleGuard = (MaleGuard*)i;
			RenderMaleGuard(maleGuard);
		}
		if (i->GetName() == "MaxGuard") {
			MaxGuard* maxGuard = (MaxGuard*)i;
			RenderMaxGuard(maxGuard);
		}
	}
}

void GameTechRenderer::RenderMaleGuard(GameObject* maleGuard) {
	RenderObject* currentRenderObject = maleGuard->GetRenderObject();
	OGLShader* shader = (OGLShader*)currentRenderObject->GetShader();
	BindShader(*shader);

	RenderObjectMaleGuard* maleGuardRenderObject = static_cast<RenderObjectMaleGuard*>(const_cast<RenderObject*>(currentRenderObject));
	Vector3 position = maleGuardRenderObject->GetMaleGuardPosition();
	Vector3 scale = maleGuardRenderObject->GetMaleGuardScale();
	Vector4 rotation = maleGuardRenderObject->GetMaleGuardRotation();
	Quaternion orientation = maleGuardRenderObject->GetMaleGuardQuaternion();

	NetworkPlayer* currentObject = (NetworkPlayer*)maleGuard;

	RenderAnimation(position, scale, rotation, orientation, currentObject->GetAnimatedObjectID(), maleGuard->GetName());
}

void GameTechRenderer::RenderMaxGuard(GameObject* maxGuard) {
	//To do

	RenderObject* currentRenderObject = maxGuard->GetRenderObject();
	OGLShader* shader = (OGLShader*)currentRenderObject->GetShader();
	BindShader(*shader);

	RenderObjectMaleGuard* maxGuardRenderObject = static_cast<RenderObjectMaleGuard*>(const_cast<RenderObject*>(currentRenderObject));
	Vector3 position = maxGuardRenderObject->GetMaleGuardPosition();
	Vector3 scale = maxGuardRenderObject->GetMaleGuardScale();
	Vector4 rotation = maxGuardRenderObject->GetMaleGuardRotation();
	Quaternion orientation = maxGuardRenderObject->GetMaleGuardQuaternion();

	MaxGuard* currentObject = (MaxGuard*)maxGuard;
	RenderAnimation(position, scale, rotation, orientation, currentObject->GetAnimatedObjectID(), maxGuard->GetName());
}

void GameTechRenderer::NewRenderLines() {
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();
	if (lines.empty()) {
		return;
	}

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());
	
	Matrix4 viewProj  = projMatrix * viewMatrix;

	BindShader(*debugShader);
	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 0);

	glUniformMatrix4fv(matSlot, 1, false, (float*)viewProj.array);

	debugLineData.clear();

	size_t frameLineCount = lines.size() * 2;

	SetDebugLineBufferSizes(frameLineCount);

	glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines.size() * sizeof(Debug::DebugLineEntry), lines.data());
	

	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, (GLsizei)frameLineCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderText() {
	const std::vector<Debug::DebugStringEntry>& strings = Debug::GetDebugStrings();
	if (strings.empty()) {
		return;
	}

	BindShader(*debugShader);

	OGLTexture* t = (OGLTexture*)Debug::GetDebugFont()->GetTexture();

	if (t) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);	
		BindTextureToShader(*t, "mainTex", 0);
	}
	Matrix4 proj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);

	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 1);

	debugTextPos.clear();
	debugTextColours.clear();
	debugTextUVs.clear();

	int frameVertCount = 0;
	for (const auto& s : strings) {
		frameVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	SetDebugStringBufferSizes(frameVertCount);

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildVerticesForString(s.data, s.position, s.colour, size, debugTextPos, debugTextUVs, debugTextColours);
	}

	glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector3), debugTextPos.data());
	glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector4), debugTextColours.data());
	glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector2), debugTextUVs.data());

	glBindVertexArray(textVAO);
	glDrawArrays(GL_TRIANGLES, 0, frameVertCount);
	glBindVertexArray(0);
}
 
Texture* GameTechRenderer::LoadTexture(const std::string& name) {
	return OGLTexture::TextureFromFile(name).release();
}

Shader* GameTechRenderer::LoadShader(const std::string& vertex, const std::string& fragment) {
	return new OGLShader(vertex, fragment);
}

void NCL::CSC8503::GameTechRenderer::ReceiveEvent(EventType eventType)
{
	switch (eventType)
	{
	case PROJECTILE_PORTAL_COLLISION:
		timeOfPortalCollision = time;
		break;
	default:
		break;
	}
}

void GameTechRenderer::SetDebugStringBufferSizes(size_t newVertCount) {
	if (newVertCount > textCount) {
		textCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector4), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector2), nullptr, GL_DYNAMIC_DRAW);

		debugTextPos.reserve(textCount);
		debugTextColours.reserve(textCount);
		debugTextUVs.reserve(textCount);

		glBindVertexArray(textVAO);

		glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, textVertVBO, 0, sizeof(Vector3));

		glVertexAttribFormat(1, 4, GL_FLOAT, false, 0);
		glVertexAttribBinding(1, 1);
		glBindVertexBuffer(1, textColourVBO, 0, sizeof(Vector4));

		glVertexAttribFormat(2, 2, GL_FLOAT, false, 0);
		glVertexAttribBinding(2, 2);
		glBindVertexBuffer(2, textTexVBO, 0, sizeof(Vector2));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::SetDebugLineBufferSizes(size_t newVertCount) {
	if (newVertCount > lineCount) {
		lineCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
		glBufferData(GL_ARRAY_BUFFER, lineCount * sizeof(Debug::DebugLineEntry), nullptr, GL_DYNAMIC_DRAW);

		debugLineData.reserve(lineCount);

		glBindVertexArray(lineVAO);

		int realStride = sizeof(Debug::DebugLineEntry) / 2;

		glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, start));
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, lineVertVBO, 0, realStride);

		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, colourA));
		glVertexAttribBinding(1, 0);
		glBindVertexBuffer(1, lineVertVBO, sizeof(Vector4), realStride);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
}


void GameTechRenderer::LoadCurrentAnimationAssets(OGLShader* currentShader, MeshMaterial* currentMaterial, MeshAnimation* currentAnimation, int animatedObjectID, int animationState) {
	anmShader = currentShader;
	
	//maleGuardMaterial = currentMaterial;
	if (maleGuardMaterial == nullptr) {
		maleGuardMaterial = new MeshMaterial("Male_Guard.mat");
	}
	if (maxGuardMaterial == nullptr) {
		maxGuardMaterial = new MeshMaterial("Rig_Maximilian.mat");
	}
	

	if (maleGuardMesh == nullptr) {
		maleGuardMesh = LoadMesh("Male_Guard.msh");
	}

	if (maxGuardMesh == nullptr) {
		maxGuardMesh = LoadMesh("Rig_Maximilian.msh");
	}

	
	if (!hasLoadedTextureToSubmesh) {
		LoadTextureToMesh();
	}
	
	if (animationStateCounter[animatedObjectID] != animationState) {
		animationStateCounter[animatedObjectID] = animationState;
		currentFrame[animatedObjectID] = 0;
	}

	if (activeAnimation[animatedObjectID] == nullptr) {
		activeAnimation[animatedObjectID] = currentAnimation;
		animationDefault[animatedObjectID] = currentAnimation;
	}
	else if (activeAnimation[animatedObjectID] != currentAnimation) {
		activeAnimation[animatedObjectID] = currentAnimation;

		//reset the currentFrame
		currentFrame[animatedObjectID] = 0;
	}


	if (normal == nullptr) {
		normal = (OGLTexture*)LoadTexture("normal.png");
	}
	if (metallic == nullptr) {
		metallic = (OGLTexture*)LoadTexture("metallic.png");
	}
	if (roughness == nullptr) {
		roughness = (OGLTexture*)LoadTexture("roughness.png");
	}
	if (ao == nullptr) {
		ao = (OGLTexture*)LoadTexture("ao.png");
	}
}



void GameTechRenderer::LoadTextureToMesh() {
	for (int i = 0; i < maleGuardMesh->GetSubMeshCount(); i++) {
		const MeshMaterialEntry* matEntry = maleGuardMaterial->GetMaterialForLayer(i);

		const string* diffusePath = nullptr;
		matEntry->GetEntry("Diffuse", &diffusePath);
		string diffuseName = *diffusePath;
		diffuseName.erase(0, 1);
		OGLTexture* diffuseTex = (OGLTexture*)LoadTexture(diffuseName);
		GLuint diffuseTexID = diffuseTex->GetObjectID();
		maleGuardMatDiffuseTextures.emplace_back(diffuseTexID);

		const string* bumpPath = nullptr;
		matEntry->GetEntry("Bump", &bumpPath);
		string bumpName = *bumpPath;
		bumpName.erase(0, 1);
		OGLTexture* bumpTex = (OGLTexture*)LoadTexture(bumpName);
		GLuint bumpTexID = bumpTex->GetObjectID();
		maleGuardMatBumpTextures.emplace_back(bumpTexID);

	}

	for (int i = 0; i < maxGuardMesh->GetSubMeshCount(); i++) {
		const MeshMaterialEntry* matEntry = maxGuardMaterial->GetMaterialForLayer(i);

		const string* diffusePath = nullptr;
		matEntry->GetEntry("Diffuse", &diffusePath);
		string diffuseName = *diffusePath;
		diffuseName.erase(0, 1);
		OGLTexture* diffuseTex = (OGLTexture*)LoadTexture(diffuseName);
		GLuint diffuseTexID = diffuseTex->GetObjectID();
		maxGuardMatDiffuseTextures.emplace_back(diffuseTexID);

		//no bump tex
		/*const string* bumpPath = nullptr;
		matEntry->GetEntry("Bump", &bumpPath);
		string bumpName = *bumpPath;
		bumpName.erase(0, 1);
		OGLTexture* bumpTex = (OGLTexture*)LoadTexture(bumpName);
		GLuint bumpTexID = bumpTex->GetObjectID();
		maxGuardMatBumpTextures.emplace_back(bumpTexID);*/

	}

	hasLoadedTextureToSubmesh = true;
}

void GameTechRenderer::Matrix4ToIdentity(Matrix4* mat4) {
	mat4->ToZero();
	mat4->array[0][0] = 1.0f;
	mat4->array[1][1] = 1.0f;
	mat4->array[2][2] = 1.0f;
	mat4->array[3][3] = 1.0f;
}

void GameTechRenderer::RenderAnimation(Vector3 inPos, Vector3 inScale, Vector4 inRotation, Quaternion inOrientation, int animatedObjectID, string name) {
	//avoid transparency
	glDisable(GL_BLEND);

	BindShader(*anmShader);
	glUniform1i(glGetUniformLocation(anmShader->GetProgramID(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(anmShader->GetProgramID(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(anmShader->GetProgramID(), "normalTex"), 2);
	//glUniform1i(glGetUniformLocation(anmShader->GetProgramID(), "albedoTex"), 3);
	glUniform1i(glGetUniformLocation(anmShader->GetProgramID(), "metallicTex"), 4);
	glUniform1i(glGetUniformLocation(anmShader->GetProgramID(), "roughnessTex"), 5);
	glUniform1i(glGetUniformLocation(anmShader->GetProgramID(), "ambiantOccTex"), 6);

	glUniform3fv(glGetUniformLocation(anmShader->GetProgramID(), "lightPos"), 1, (float*)&lightPosition);
	glUniform4fv(glGetUniformLocation(anmShader->GetProgramID(), "lightColour"), 1, (float*)&lightColour);
	glUniform1f(glGetUniformLocation(anmShader->GetProgramID(), "lightRadius"), lightRadius);

	Vector3 camPos = gameWorld.GetMainCamera().GetPosition();
	glUniform3fv(glGetUniformLocation(anmShader->GetProgramID(), "cameraPos"), 1, (float*)&camPos.x);

	Matrix4 modelMatrix;
	Matrix4ToIdentity(&modelMatrix);
	/*modelMatrix = 
		Matrix4::Translation(inPos) * 
		Matrix4::Scale(inScale) * 
		Matrix4::Rotation(inRotation.x, Vector3(inRotation.y,inRotation.z,inRotation.w));*/
	modelMatrix =
		Matrix4::Translation(inPos) *
		Matrix4(inOrientation) *
		Matrix4::Scale(inScale);
	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	glUniformMatrix4fv(glGetUniformLocation(anmShader->GetProgramID(), "modelMatrix"), 1, false, (float*)&modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(anmShader->GetProgramID(), "viewMatrix"), 1, false, (float*)&viewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(anmShader->GetProgramID(), "projMatrix"), 1, false, (float*)&projMatrix);
	
	vector<Matrix4> frameMatrices;
	maleGuardMesh->CalculateInverseBindPose();
	const vector<Matrix4> invBindPoseMaleGuard = maleGuardMesh->GetInverseBindPose();
	maxGuardMesh->CalculateInverseBindPose();
	const vector<Matrix4> invBindPoseMaxGuard = maxGuardMesh->GetInverseBindPose();

	const Matrix4* frameDataAnm = activeAnimation[animatedObjectID]->GetJointData(currentFrame[animatedObjectID]);

	if (frameDataAnm != nullptr) {
		if (name == "MaleGuard") {
			for (GLuint i = 0; i < maleGuardMesh->GetJointCount(); i++) {
				frameMatrices.emplace_back(frameDataAnm[i] * invBindPoseMaleGuard[i]);
			}
		}
		if (name == "MaxGuard") {
			for (GLuint i = 0; i < maxGuardMesh->GetJointCount(); i++) {
				frameMatrices.emplace_back(frameDataAnm[i] * invBindPoseMaxGuard[i]);
			}
		}
	}
	/*for (GLuint i = 0; i < maleGuardMesh->GetJointCount(); i++) {
		frameMatrices.emplace_back(frameDataAnm[i] * invBindPose[i]);
	}*/

	int	shaderLocation = glGetUniformLocation(anmShader->GetProgramID(), "joints");
	glUniformMatrix4fv(shaderLocation, frameMatrices.size(), false, (float*)frameMatrices.data());

	//use diffuseTex as albedo
	/*glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, albedo->GetObjectID());*/
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normal->GetObjectID());
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, metallic->GetObjectID());
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, roughness->GetObjectID());
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, ao->GetObjectID());

	if (name == "MaleGuard") {
		for (int i = 0; i < maleGuardMesh->GetSubMeshCount(); i++) {

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, maleGuardMatDiffuseTextures[i]);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, maleGuardMatBumpTextures[i]);

			BindMesh((OGLMesh&)*maleGuardMesh);
			DrawBoundMesh((uint32_t)i);
		}
	}
	if (name == "MaxGuard") {
		for (int i = 0; i < maxGuardMesh->GetSubMeshCount(); i++) {

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, maxGuardMatDiffuseTextures[i]);

			/*glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, maxGuardMatBumpTextures[i]);*/

			BindMesh((OGLMesh&)*maxGuardMesh);
			DrawBoundMesh((uint32_t)i);
		}
	}

	//test
	

	/*if (name == "MaleGuard") {
		for (int i = 0; i < maleGuardMesh->GetSubMeshCount(); i++) {

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, maleGuardMatDiffuseTextures[i]);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, maleGuardMatBumpTextures[i]);

			const SubMesh* pose = maleGuardMesh->GetSubMesh(i);

			vector<Matrix4> frameMatrices;

			for (unsigned int k; k < pose->count; k++) {
				int jointID = 
			}


			BindMesh((OGLMesh&)*maleGuardMesh);
			DrawBoundMesh((uint32_t)i);
		}
	}*/
	//test

	glEnable(GL_BLEND);
}

void GameTechRenderer::Update(float dt) {
	gameWorld.gameObjectsMutex.lock();

	time += dt;

	for (int i = 0; i < activeAnimatedObjects.size(); i++) {
		frameTime[i] -= dt;
		while (frameTime[i] < 0.0f) {
			if (currentFrame[i] + 1 >= activeAnimation[i]->GetFrameCount()) {

				((NetworkPlayer*)activeAnimatedObjects[i])->SetAnimation(AnmName::STEPFORWARD);
			}
			currentFrame[i] = (currentFrame[i] + 1) % activeAnimation[i]->GetFrameCount();
			frameTime[i] += 1.0f / activeAnimation[i]->GetFrameTime();
		}
	}
	gameWorld.gameObjectsMutex.unlock();
}