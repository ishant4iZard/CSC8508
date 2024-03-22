#include "GameTechAGCRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"
#include "../PS5Core/AGCMesh.h"
#include "../PS5Core/AGCTexture.h"
#include "../PS5Core/AGCShader.h"

#include "../CSC8503CoreClasses/Debug.h"

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;
using namespace PS5;

const int SHADOW_SIZE = 8192;
const int FRAMES_IN_FLIGHT = 2;

const int BINDLESS_TEX_COUNT = 128;
const int BINDLESS_BUFFER_COUNT = 128;

const size_t LINE_STRIDE = sizeof(Vector4) + sizeof(Vector4);
const size_t TEXT_STRIDE = sizeof(Vector2) + sizeof(Vector2) + sizeof(Vector4);

GameTechAGCRenderer::GameTechAGCRenderer(GameWorld& world) : AGCRenderer(*Window::GetWindow()), gameWorld(world) {
	SceError error = SCE_OK;
	bindlessTextures = (sce::Agc::Core::Texture*)allocator.Allocate(BINDLESS_TEX_COUNT * sizeof(sce::Agc::Core::Texture), sce::Agc::Alignment::kBuffer);
	sce::Agc::Core::BufferSpec texSpec;
	texSpec.initAsRegularBuffer(bindlessTextures, sizeof(sce::Agc::Core::Texture), BINDLESS_TEX_COUNT);
	error = sce::Agc::Core::initialize(&textureBuffer, &texSpec);

	bindlessBuffers = (sce::Agc::Core::Buffer*)allocator.Allocate(BINDLESS_BUFFER_COUNT * sizeof(sce::Agc::Core::Buffer), sce::Agc::Alignment::kBuffer);
	sce::Agc::Core::BufferSpec buffSpec;
	buffSpec.initAsRegularBuffer(bindlessBuffers, sizeof(sce::Agc::Core::Buffer), BINDLESS_BUFFER_COUNT);
	error = sce::Agc::Core::initialize(&arrayBuffer, &buffSpec);
	bufferCount = 1; //We skip over index 0, makes some selection logic easier later

	rendererModel = new GameTechAGCModel();

	rendererModel->SetDefaultTex((AGCTexture*)LoadTexture("doge.png"));
	rendererModel->SetSkyboxTex((AGCTexture*)LoadTexture("Skybox.dds"));

	quadMesh = new AGCMesh();
	CreateQuad(quadMesh);
	quadMesh->UploadToGPU(this);

	rendererModel->SetComputeShader(ComputeShaderName::SKINNING, new AGCShader("Skinning_c.ags", allocator));
	rendererModel->SetComputeShader(ComputeShaderName::GAMMA, new AGCShader("Gamma_c.ags", allocator));

	rendererModel->SetVertexShader(VertexShaderName::DEFAULT, new AGCShader("Tech_vv.ags", allocator));
	rendererModel->SetPixelShader(PixelShaderName::DEFAULT, new AGCShader("Tech_p.ags", allocator));

	rendererModel->SetVertexShader(VertexShaderName::SHADOW, new AGCShader("Shadow_vv.ags", allocator));
	rendererModel->SetPixelShader(PixelShaderName::SHADOW, new AGCShader("Shadow_p.ags", allocator));

	rendererModel->SetVertexShader(VertexShaderName::SKY_BOX, new AGCShader("Skybox_vv.ags", allocator));
	rendererModel->SetPixelShader(PixelShaderName::SKY_BOX, new AGCShader("Skybox_p.ags", allocator));

	rendererModel->SetVertexShader(VertexShaderName::DEBUG_LINE, new AGCShader("DebugLine_vv.ags", allocator));
	rendererModel->SetPixelShader(PixelShaderName::DEBUG_LINE, new AGCShader("DebugLine_p.ags", allocator));

	rendererModel->SetVertexShader(VertexShaderName::DEBUG_TEXT, new AGCShader("DebugText_vv.ags", allocator));
	rendererModel->SetPixelShader(PixelShaderName::DEBUG_TEXT, new AGCShader("DebugText_p.ags", allocator));

	rendererModel->SetVertexShader(VertexShaderName::PBR ,new AGCShader("PBR_vv.ags", allocator));
	rendererModel->SetPixelShader(PixelShaderName::PBR, new AGCShader("PBR_p.ags", allocator));

	allFrames = new FrameData[FRAMES_IN_FLIGHT];
	// Initialize framebuffer constant buffer to pass constant data among shader
	for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {

		{//We store scene object matrices etc in a big UBO
			allFrames[i].data.dataStart = (char*)allocator.Allocate(1024 * 1024 * 64, sce::Agc::Alignment::kBuffer);
			allFrames[i].data.data = allFrames[i].data.dataStart;

			sce::Agc::Core::BufferSpec tempConstBuffSpec;
			tempConstBuffSpec.initAsConstantBuffer(allFrames[i].data.dataStart, sizeof(ShaderConstants));

			SceError error = sce::Agc::Core::initialize(&allFrames[i].constantBuffer, &tempConstBuffSpec);
		}
	}

	currentFrameIndex = 0;
	currentFrame = &allFrames[currentFrameIndex];

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadTexture("PressStart2P.png"));

	shadowTarget = CreateDepthBufferTarget(SHADOW_SIZE, SHADOW_SIZE);
	shadowMap = CreateFrameBufferTextureSlot("Shadowmap");

	screenTarget = CreateColourBufferTarget(Window::GetWindow()->GetScreenSize().x, Window::GetWindow()->GetScreenSize().y, true);
	screenTex = CreateFrameBufferTextureSlot("Screen");

	error = sce::Agc::Core::translate(screenTex->GetAGCPointer(), &screenTarget, sce::Agc::Core::RenderTargetComponent::kData);

	InitPbrSamplers();

	shadowSampler.init()
		.setXyFilterMode(
			sce::Agc::Core::Sampler::FilterMode::kPoint,	//magnification
			sce::Agc::Core::Sampler::FilterMode::kPoint		//minificaction
		)
		.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);
}

GameTechAGCRenderer::~GameTechAGCRenderer() {
	delete rendererModel;
}

Mesh* GameTechAGCRenderer::LoadMesh(const std::string& name) {
	AGCMesh* m = new AGCMesh();

	if (name.find(".gltf") != std::string::npos) {
		bool a = true;
	}
	else if (name.find(".msh") != std::string::npos) {
		MshLoader::LoadMesh(name, *m);
	}

	m->UploadToGPU(this);

	return m;
}

NCL::PS5::AGCTexture* GameTechAGCRenderer::CreateFrameBufferTextureSlot(const std::string& name) {
	uint32_t index = textureMap.size();
	AGCTexture* t = new AGCTexture(allocator);

	textureMap.insert({ name, t });
	t->SetAssetID(index);
	bindlessTextures[t->GetAssetID()] = *t->GetAGCPointer();

	return t;
}

Texture* GameTechAGCRenderer::LoadTexture(const std::string& name) {
	auto found = textureMap.find(name);
	if (found != textureMap.end()) {
		return (Texture*)found->second;
	}
	AGCTexture* t = new AGCTexture(name, allocator);

	t->SetAssetID(textureMap.size());
	textureMap.insert({ name, t });

	bindlessTextures[t->GetAssetID()] = *t->GetAGCPointer();

	return (Texture*)t;
}

Shader* GameTechAGCRenderer::LoadShader(const std::string& vertex, const std::string& fragment) {
	return nullptr;
}

void GameTechAGCRenderer::RenderFrame() {
	currentFrame = &allFrames[currentFrameIndex]; //Get data from back buffer to write on

	currentFrame->data.Reset();

	currentFrame->globalDataOffset = 0;
	currentFrame->objectStateOffset = sizeof(ShaderConstants);
	currentFrame->debugLinesOffset = currentFrame->objectStateOffset; //We'll be pushing that out later

	//Step 1: Write the frame's constant data to the buffer
	WriteRenderPassConstants();
	//Step 2: Walk the object list and build up the object set and required buffer memory
	UpdateObjectList();
	//Step 3: Run a compute shader for every skinned mesh to generate its positions, tangents, and normals
	GPUSkinningPass();
	//Step 4: Go through the geometry and darw it to a shadow map
	ShadowmapPass();
	//Step 5: Draw a skybox to our main scene render target
	SkyboxPass();
	//Step 6: Draw the scene to our main scene render target
	MainRenderPass();
	//Step 7: Draw the debug data to the main scene render target
	UpdateDebugData();
	RenderDebugLines();
	RenderDebugText();
	//Step 8: Draw the main scene render target to the screen with a compute shader
	DisplayRenderPass(); //Puts our scene on screen, uses a compute

	currentFrameIndex = (currentFrameIndex + 1) % FRAMES_IN_FLIGHT; //Swap the front and back buffer
}

/*
This method builds a struct that

*/
void GameTechAGCRenderer::WriteRenderPassConstants() {
	ShaderConstants tempConstantFrameData;
	tempConstantFrameData.lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	tempConstantFrameData.lightRadius = 1000.0f;
	tempConstantFrameData.lightPosition = Vector3(0.0f, 100.0f, 0.0f);
	tempConstantFrameData.cameraPos = gameWorld.GetMainCamera().GetPosition();

	tempConstantFrameData.viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	tempConstantFrameData.projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	tempConstantFrameData.viewProjMatrix = tempConstantFrameData.projMatrix * tempConstantFrameData.viewMatrix;

	tempConstantFrameData.inverseViewProjMatrix = tempConstantFrameData.viewProjMatrix.Inverse();
	tempConstantFrameData.inverseViewMatrix = tempConstantFrameData.viewMatrix.Inverse();
	tempConstantFrameData.inverseProjMatrix = tempConstantFrameData.projMatrix.Inverse();

	tempConstantFrameData.orthoMatrix = Matrix4::Orthographic(0.0f, 100.0f, 100.0f, 0.0f, -1.0f, 1.0f);
	tempConstantFrameData.shadowMatrix = Matrix4::Perspective(50.0f, 500.0f, 1.0f, 45.0f) *
		Matrix4::BuildViewMatrix(tempConstantFrameData.lightPosition, Vector3(0, 0, 0), Vector3(0, 1, 0));

	tempConstantFrameData.shadowID = shadowMap->GetAssetID();

	currentFrame->data.WriteData<ShaderConstants>(tempConstantFrameData); //Let's start filling up our frame data!

	currentFrame->data.AlignData(128);
	currentFrame->objectStateOffset = currentFrame->data.bytesWritten;
}

void GameTechAGCRenderer::DrawObjects() {
	if (activeObjects.empty()) {
		return;
	}
	uint32_t startingIndex = 0;

	AGCMesh* prevMesh = (AGCMesh*)activeObjects[0]->GetMesh();
	int instanceCount = 0;

	for (int i = 0; i < activeObjects.size(); ++i) {
		AGCMesh* objectMesh = (AGCMesh*)activeObjects[i]->GetMesh();

		//The new mesh is different than previous meshes, flush out the old list
		if (prevMesh != objectMesh) {
			prevMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));

			uint32_t* objID = static_cast<uint32_t*>(frameContext->m_dcb.allocateTopDown(sizeof(uint32_t), sce::Agc::Alignment::kBuffer));
			*objID = startingIndex;
			frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(objID, 1);

			DrawBoundMeshInstanced(*frameContext, *prevMesh, instanceCount);
			prevMesh = objectMesh;
			instanceCount = 0;
			startingIndex = i;
		}
		if (i == activeObjects.size() - 1) {
			objectMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));

			uint32_t* objID = static_cast<uint32_t*>(frameContext->m_dcb.allocateTopDown(sizeof(uint32_t), sce::Agc::Alignment::kBuffer));
			*objID = startingIndex;
			frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(objID, 1);

			if (prevMesh == objectMesh) {
				instanceCount++;
			}

			DrawBoundMeshInstanced(*frameContext, *objectMesh, instanceCount);
		}
		else {
			instanceCount++;
		}
	}
}

void GameTechAGCRenderer::GPUSkinningPass() {
	if (frameJobs.empty()) {
		return;
	}

	frameContext->setCsShader(rendererModel->GetComputeShader(ComputeShaderName::SKINNING)->GetAGCPointer());

	sce::Agc::DispatchModifier modifier = rendererModel->GetComputeShader(ComputeShaderName::SKINNING)->GetAGCPointer()->m_specials->m_dispatchModifier;

	for (auto& i : frameJobs) {
		NCL::PS5::AGCMesh* m = (AGCMesh*)i.object->GetMesh();

		sce::Agc::Core::Buffer inputBuffers[6];

		if (!m->GetAGCBuffer(VertexAttribute::Positions, inputBuffers[0]) ||
			!m->GetAGCBuffer(VertexAttribute::Normals, inputBuffers[1]) ||
			!m->GetAGCBuffer(VertexAttribute::Tangents, inputBuffers[2]) ||
			!m->GetAGCBuffer(VertexAttribute::JointWeights, inputBuffers[3]) ||
			!m->GetAGCBuffer(VertexAttribute::JointIndices, inputBuffers[4])) {
			continue;
		}
		char* offset = currentFrame->data.data;

		std::vector<Matrix4>& skeleton = i.object->GetSkeleton();
		currentFrame->data.WriteData(skeleton.data(), sizeof(Matrix4) * skeleton.size());

		sce::Agc::Core::BufferSpec bufSpec;
		bufSpec.initAsRegularBuffer(offset, sizeof(Matrix4), skeleton.size());
		SceError error = sce::Agc::Core::initialize(&inputBuffers[5], &bufSpec);

		inputBuffers[5].setFormat(sce::Agc::Core::Buffer::Format::k32_32_32_32Float);

		frameContext->m_bdr.getStage(sce::Agc::ShaderType::kCs)
			.setBuffers(0, 6, inputBuffers)
			.setRwBuffers(0, 1, &bindlessBuffers[i.outputIndex]);

		uint32_t threadCount = (m->GetVertexCount() + 63) / 64;
		frameContext->m_dcb.dispatch(threadCount, 1, 1, modifier);
	}
	//TODO fence
	frameJobs.clear();

	frameContext->setCsShader(nullptr);
}

void NCL::CSC8503::GameTechAGCRenderer::InitPbrSamplers()
{
	normalSampler.init()
		.setXyFilterMode(
			sce::Agc::Core::Sampler::FilterMode::kPoint,			//magnification
			sce::Agc::Core::Sampler::FilterMode::kPoint		//minificaction
		)
		.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);

	roughnessSampler.init()
		.setXyFilterMode(
			sce::Agc::Core::Sampler::FilterMode::kPoint,			//magnification
			sce::Agc::Core::Sampler::FilterMode::kPoint		//minificaction
		)
		.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);

	mettalicSampler.init()
		.setXyFilterMode(
			sce::Agc::Core::Sampler::FilterMode::kPoint,			//magnification
			sce::Agc::Core::Sampler::FilterMode::kPoint		//minificaction
		)
		.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);

	aoSampler.init()
		.setXyFilterMode(
			sce::Agc::Core::Sampler::FilterMode::kPoint,			//magnification
			sce::Agc::Core::Sampler::FilterMode::kPoint		//minificaction
		)
		.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);
}


void GameTechAGCRenderer::SkyboxPass() {
	frameContext->setShaders(nullptr,
		rendererModel->GetVertexShader(VertexShaderName::SKY_BOX)->GetAGCPointer(),
		rendererModel->GetPixelShader(PixelShaderName::SKY_BOX)->GetAGCPointer(),
		sce::Agc::UcPrimitiveType::Type::kTriList);

	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SCREENWIDTH, SCREENHEIGHT, 0, 0, -1.0f, 1.0f);
	frameContext->m_sb.setState(viewPort);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(screenTarget);

	frameContext->m_sb.setState(depthTarget);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable);
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kAlways);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &defaultSampler)
		.setTextures(1, 1, rendererModel->GetSkyboxTex()->GetAGCPointer());

	quadMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMesh(*frameContext, *quadMesh);
}

void GameTechAGCRenderer::ShadowmapPass() {
	sce::Agc::Toolkit::Result tk1 = sce::Agc::Toolkit::clearDepthRenderTargetCs(&frameContext->m_dcb, &shadowTarget);
	sce::Agc::Toolkit::Result wat = frameContext->resetToolkitChangesAndSyncToGl2(tk1);

	frameContext->setShaders(
		nullptr,
		rendererModel->GetVertexShader(VertexShaderName::SHADOW)->GetAGCPointer(),
		rendererModel->GetPixelShader(PixelShaderName::SHADOW)->GetAGCPointer(),
		sce::Agc::UcPrimitiveType::Type::kTriList);

	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SHADOW_SIZE, SHADOW_SIZE, 0, 0, -1.0f, 1.0f);
	frameContext->m_sb.setState(viewPort);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0x0);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(shadowTarget);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl
		.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable)
		.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLessEqual)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kEnable);

	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->objectBuffer)
		.setBuffers(1, 1, &arrayBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &defaultSampler)
		.setBuffers(1, 1, &textureBuffer);

	DrawObjects();

	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushCompressedDepthBufferForTexture);

	const bool htileTC = (shadowTarget.getTextureCompatiblePlaneCompression() == sce::Agc::CxDepthRenderTarget::TextureCompatiblePlaneCompression::kEnable);
	const sce::Agc::Core::MaintainCompression maintainCompression = htileTC ? sce::Agc::Core::MaintainCompression::kEnable : sce::Agc::Core::MaintainCompression::kDisable;

	sce::Agc::Core::translate(&bindlessTextures[shadowMap->GetAssetID()], &shadowTarget, sce::Agc::Core::DepthRenderTargetComponent::kDepth, maintainCompression);
}

void GameTechAGCRenderer::MainRenderPass() {
	frameContext->setShaders(nullptr, rendererModel->GetVertexShader(VertexShaderName::PBR)->GetAGCPointer(), rendererModel->GetPixelShader(PixelShaderName::PBR)->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);

	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SCREENWIDTH, SCREENHEIGHT, 0, 0, -1.0f, 1.0f);
	frameContext->m_sb.setState(viewPort);
	frameContext->m_sb.setState(backBuffers[currentSwap].targetMask);
	frameContext->m_sb.setState(backBuffers[currentSwap].renderTarget);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(screenTarget);

	frameContext->m_sb.setState(depthTarget);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init();
	blendControl.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setAlphaBlendFunc(sce::Agc::CxBlendControl::AlphaBlendFunc::kAdd)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
		.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd);

	frameContext->m_sb.setState(blendControl);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable);
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLessEqual);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kEnable);

	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->objectBuffer)
		.setBuffers(1, 1, &arrayBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &textureBuffer)
		.setSamplers(0, 1, &defaultSampler)
		.setSamplers(1, 1, &normalSampler)
		.setSamplers(2, 1, &roughnessSampler)
		.setSamplers(3, 1, &mettalicSampler)
		.setSamplers(4, 1, &aoSampler)
		.setSamplers(5, 1, &shadowSampler);
	DrawObjects();
}

void GameTechAGCRenderer::UpdateDebugData() {
	const std::vector<NCL::Debug::DebugStringEntry>& strings = NCL::Debug::GetDebugStrings();
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();

	currentFrame->textVertCount = 0;
	currentFrame->lineVertCount = 0;

	for (const auto& s : strings) {
		currentFrame->textVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	currentFrame->lineVertCount = (int)lines.size() * 2;

	currentFrame->debugLinesOffset = currentFrame->data.bytesWritten;

	currentFrame->data.WriteData((void*)lines.data(), (size_t)currentFrame->lineVertCount * LINE_STRIDE);

	currentFrame->debugTextOffset = currentFrame->data.bytesWritten;
	std::vector< NCL::Rendering::SimpleFont::InterleavedTextVertex> verts;

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildInterleavedVerticesForString(s.data, s.position, s.colour, size, verts);
		//can now copy to GPU visible mem
		size_t count = verts.size() * TEXT_STRIDE;
		memcpy(currentFrame->data.data, verts.data(), count);
		currentFrame->data.data += count;
		currentFrame->data.bytesWritten += count;
		verts.clear();
	}
}

void GameTechAGCRenderer::DisplayRenderPass() {
	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushUncompressedColorBufferForTexture);

	frameContext->setCsShader(rendererModel->GetComputeShader(ComputeShaderName::GAMMA)->GetAGCPointer());

	sce::Agc::DispatchModifier modifier = rendererModel->GetComputeShader(ComputeShaderName::GAMMA)->GetAGCPointer()->m_specials->m_dispatchModifier;

	sce::Agc::Core::Texture outputTex; //Alias for our framebuffer tex;
	SceError error = sce::Agc::Core::translate(&outputTex, &backBuffers[currentSwap].renderTarget, sce::Agc::Core::RenderTargetComponent::kData);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kCs)
		.setTextures(0, 1, screenTex->GetAGCPointer())
		.setRwTextures(1, 1, &outputTex);
	uint32_t xDims = (outputTex.getWidth() + 7) / 8;
	uint32_t yDims = (outputTex.getHeight() + 7) / 8;

	frameContext->m_dcb.dispatch(xDims, yDims, 1, modifier);
}

void GameTechAGCRenderer::RenderDebugLines() {
	if (currentFrame->lineVertCount == 0) {
		return;
	}
	frameContext->setShaders(nullptr,
		rendererModel->GetVertexShader(VertexShaderName::DEBUG_LINE)->GetAGCPointer(),
		rendererModel->GetPixelShader(PixelShaderName::DEBUG_LINE)->GetAGCPointer(),
		sce::Agc::UcPrimitiveType::Type::kLineList);
	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	char* dataPos = currentFrame->data.dataStart + currentFrame->debugLinesOffset;
	size_t dataCount = currentFrame->lineVertCount;

	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(dataPos, LINE_STRIDE, dataCount);
	SceError error = sce::Agc::Core::initialize(&currentFrame->debugLineBuffer, &bufSpec);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->debugLineBuffer);

	frameContext->drawIndexAuto(currentFrame->lineVertCount);
}

void GameTechAGCRenderer::RenderDebugText() {
	if (currentFrame->textVertCount == 0) {
		return;
	}
	frameContext->setShaders(nullptr,
		rendererModel->GetVertexShader(VertexShaderName::DEBUG_TEXT)->GetAGCPointer(),
		rendererModel->GetPixelShader(PixelShaderName::DEBUG_TEXT)->GetAGCPointer(),
		sce::Agc::UcPrimitiveType::Type::kTriList);
	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init();
	blendControl.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setAlphaBlendFunc(sce::Agc::CxBlendControl::AlphaBlendFunc::kAdd)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
		.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd);

	frameContext->m_sb.setState(blendControl);

	char* dataPos = currentFrame->data.dataStart + currentFrame->debugTextOffset;
	size_t dataCount = currentFrame->textVertCount;

	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(dataPos, TEXT_STRIDE, dataCount);
	SceError error = sce::Agc::Core::initialize(&currentFrame->debugTextBuffer, &bufSpec);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->debugTextBuffer);

	AGCTexture* debugTex = (AGCTexture*)Debug::GetDebugFont()->GetTexture();

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &pixelSampler)
		.setTextures(1, 1, debugTex->GetAGCPointer());

	frameContext->drawIndexAuto(currentFrame->textVertCount);
}

void GameTechAGCRenderer::UpdateObjectList() {
	activeObjects.clear();

	char* dataPos = currentFrame->data.data; //Get starting position of last data entered
	int at = 0;
	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				RenderObject* g = o->GetRenderObject();
				if (g) {
					activeObjects.push_back(g);

					ObjectState state; //PSSL Header
					state.modelMatrix = g->GetTransform()->GetMatrix();
					state.inverseModelMatrix = g->GetTransform()->GetMatrix().Inverse();

					state.colour = g->GetColour();
					state.tiling = g->GetTiling();

					state.index[0] = 0; //Default Texture
					state.index[1] = 0; //Skinning buffer
					Texture* t = g->GetDefaultTexture();
					if (t) {
						state.index[0] = t->GetAssetID();
					}

					SetPbrTexture(&state, g);

					AGCMesh* m = (AGCMesh*)g->GetMesh();
					if (m && m->GetJointCount() > 0) {//It's a skeleton mesh, need to update transformed vertices buffer

						Buffer* b = g->GetGPUBuffer();
						if (!b) {
							//We've not yet made a buffer to hold the verts of this mesh!
							//We need a new mesh to store the positions, normals, and tangents of this mesh
							size_t vertexSize = sizeof(Vector3) + sizeof(Vector3) + sizeof(Vector4); //Position + Scale + Rotation gives size of one vertex
							size_t vertexCount = m->GetVertexCount();
							size_t bufferSize = vertexCount * vertexSize; //Total size of buffer to hold data of mesh vertices

							char* vertexData = (char*)allocator.Allocate((uint64_t)(bufferSize), sce::Agc::Alignment::kBuffer);

							sce::Agc::Core::BufferSpec tempRegularBufferSpecs;
							tempRegularBufferSpecs.initAsRegularBuffer(vertexData, vertexSize, vertexCount);

							sce::Agc::Core::Buffer tempVertexBuffer;
							SceError error = sce::Agc::Core::initialize(&tempVertexBuffer, &tempRegularBufferSpecs);

							uint32_t bufferID = bufferCount++;
							b = new AGCBuffer(tempVertexBuffer, vertexData);
							b->SetAssetID(bufferID);
							g->SetGPUBuffer(b);

							bindlessBuffers[bufferID] = tempVertexBuffer;
						}
						state.index[1] = b->GetAssetID();

						frameJobs.push_back({ g, b->GetAssetID() });
					}
					currentFrame->data.WriteData<ObjectState>(state);
					currentFrame->debugLinesOffset += sizeof(ObjectState);
					at++;
				}
			}
		}
	);

	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(dataPos, sizeof(ObjectState), at);
	sce::Agc::Core::initialize(&currentFrame->objectBuffer, &bufSpec);
}

void NCL::CSC8503::GameTechAGCRenderer::SetPbrTexture(ObjectState* outState, RenderObject* inRenderObj)
{
	AGCTexture* tempTex = (AGCTexture*)inRenderObj->GetTexture(TextureType::ALBEDO);
	if (tempTex)
	{
		outState->index[0] = tempTex->GetAssetID();
		outState->index[2] = inRenderObj->GetTexture(TextureType::NORMAL)->GetAssetID();
		outState->index[3] = inRenderObj->GetTexture(TextureType::ROUGHNESS)->GetAssetID();
		outState->index[4] = inRenderObj->GetTexture(TextureType::METAL)->GetAssetID();
		outState->index[5] = inRenderObj->GetTexture(TextureType::AO)->GetAssetID();
	}
}
