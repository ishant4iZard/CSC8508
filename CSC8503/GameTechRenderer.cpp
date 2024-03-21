#include "GameTechRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"
#include "DirectionalLight.h"
#include "OglHdrFbo.h"
#include "OglPostProcessingFbo.h"

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
	particleShader = new OGLShader("particleDefault.vert", "particleMoving.frag");

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

	ui->RenderUI();
}

void GameTechRenderer::BuildObjectList() {
	activeObjects.clear();
	instancedRenderObjectList.clear();
	activeAnimatedObjects.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o && o->IsActive()) {
				const RenderObject* g = o->GetRenderObject();

				if (o->gettag() == "Player") {
					activeAnimatedObjects.emplace_back(o);
				}

				if (g) {
					/*if (o->gettag() == "Player") {
						activeAnimatedObjects.emplace_back(g);
					}*/

					if (g->GetMesh()->GetInstanceCount() > 0)
						instancedRenderObjectList.emplace_back(g);
					else
						activeObjects.emplace_back(g);
				}
			}
		}
	);
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
	RenderAnimatedObjects();
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

MeshMaterial* GameTechRenderer::LoadMaterial(const std::string& name) {
	return new MeshMaterial(name);
}

MeshAnimation* GameTechRenderer::LoadAnimation(const std::string& name) {
	return new MeshAnimation(name);
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

void GameTechRenderer::Matrix4ToIdentity(Matrix4* mat4) {
	mat4->ToZero();
	mat4->array[0][0] = 1.0f;
	mat4->array[1][1] = 1.0f;
	mat4->array[2][2] = 1.0f;
	mat4->array[3][3] = 1.0f;
}

void GameTechRenderer::RenderAnimatedObjects() {
	glDisable(GL_BLEND);

	OGLShader* activeShader = nullptr;

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	for (const auto& tempAnimatedRenderObject : activeAnimatedObjects) {
		OGLShader* shader = (OGLShader*)(*tempAnimatedRenderObject).GetRenderObject()->GetShader();
		BindShader(*shader);

		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "diffuseTex"), 6);
		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "bumpTex"), 7);
		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "normalTex"), 2);
		//glUniform1i(glGetUniformLocation(shader->GetProgramID(), "albedoTex"), 3);
		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "metallicTex"), 3);
		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "roughnessTex"), 4);
		glUniform1i(glGetUniformLocation(shader->GetProgramID(), "ambiantOccTex"), 5);

		glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "lightPos"), 1, (float*)&lightPosition);
		glUniform4fv(glGetUniformLocation(shader->GetProgramID(), "lightColour"), 1, (float*)&lightColour);
		glUniform1f(glGetUniformLocation(shader->GetProgramID(), "lightRadius"), lightRadius);

		Vector3 camPos = gameWorld.GetMainCamera().GetPosition();
		glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "cameraPos"), 1, (float*)&camPos.x);

		Matrix4 modelMatrix;
		Matrix4ToIdentity(&modelMatrix);
		/*modelMatrix =
			Matrix4::Translation(inPos) *
			Matrix4(inOrientation) *
			Matrix4::Scale(inScale);*/

		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "modelMatrix"), 1, false, (float*)&modelMatrix);
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "viewMatrix"), 1, false, (float*)&viewMatrix);
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "projMatrix"), 1, false, (float*)&projMatrix);

		//use diffuseTex as albedo

		RenderObject* renderObject = tempAnimatedRenderObject->GetRenderObject();
		UpdatePBRUniforms(renderObject);

		//GLuint albedo		= ((OGLTexture*)renderObject->GetTexture(static_cast<TextureType>(0)))->GetObjectID();
		//GLuint normal		= ((OGLTexture*)renderObject->GetTexture(static_cast<TextureType>(1)))->GetObjectID();
		GLuint metallic		= ((OGLTexture*)renderObject->GetTexture(static_cast<TextureType>(2)))->GetObjectID();
		GLuint roughness	= ((OGLTexture*)renderObject->GetTexture(static_cast<TextureType>(3)))->GetObjectID();
		GLuint ao			= ((OGLTexture*)renderObject->GetTexture(static_cast<TextureType>(4)))->GetObjectID();

		//glActiveTexture(GL_TEXTURE3);
		//glBindTexture(GL_TEXTURE_2D, albedo);
		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, normal->GetObjectID());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, metallic);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, roughness);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, ao);



		int	jointsLocation = glGetUniformLocation(shader->GetProgramID(), "joints");
		
		vector<Matrix4> frameMatrices = tempAnimatedRenderObject->GetRenderObject()->GetSkeleton();

		glUniformMatrix4fv(jointsLocation, frameMatrices.size(), false, (float*)frameMatrices.data());

		Mesh* tempMesh = tempAnimatedRenderObject->GetRenderObject()->GetMesh();

		Texture* diffuseTex[4];
		Texture* bumpTex[4];

		for (int i = 0; i < 4; ++i) {
			diffuseTex[i] = tempAnimatedRenderObject->GetRenderObject()->GetTextureAnm("Diffuse", i);
			bumpTex[i] = tempAnimatedRenderObject->GetRenderObject()->GetTextureAnm("Bump", i);
		}

		/*tempAnimatedRenderObject->GetRenderObject()->GetTextureAnm("Diffuse", diffuseTex);
		tempAnimatedRenderObject->GetRenderObject()->GetTextureAnm("Bump", bumpTex);*/

		for (int i = 0; i < tempMesh->GetSubMeshCount(); ++i) {
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)diffuseTex[i])->GetObjectID());

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, ((OGLTexture*)bumpTex[i])->GetObjectID());

			BindMesh((OGLMesh&)*(tempMesh));
			DrawBoundMesh((uint32_t)i);
		}
	}

	glEnable(GL_BLEND);
}