#include "GameTechVulkanRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "VulkanUtils.h"
#include "MshLoader.h"
#ifdef USEVULKAN
using namespace NCL;
using namespace Rendering;
using namespace CSC8503;
using namespace Vulkan;

const int _SHADOWSIZE = 8192;
const int _FRAMECOUNT = 3;

const int _TEXCOUNT = 128;

const size_t lineStride = sizeof(Vector4) + sizeof(Vector4);
const size_t textStride = sizeof(Vector2) + sizeof(Vector4) + sizeof(Vector2);

GameTechVulkanRenderer::GameTechVulkanRenderer(GameWorld& world) : VulkanRenderer(*Window::GetWindow()), gameWorld(world) {
	majorVersion = 1;
	minorVersion = 3;
	autoBeginDynamicRendering = false;
}

GameTechVulkanRenderer::~GameTechVulkanRenderer() {

}

void GameTechVulkanRenderer::SetupDevice(vk::PhysicalDeviceFeatures2& deviceFeatures) {
	allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	deviceExtensions.emplace_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	deviceExtensions.emplace_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

	static vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR deviceAddressfeature(true);
	deviceFeatures.pNext = &deviceAddressfeature;
}

void	GameTechVulkanRenderer::InitStructures() {
	globalDataLayout = DescriptorSetLayoutBuilder(GetDevice())
		.WithUniformBuffers(1)	//0: The global data uniform buffer
		.WithStorageBuffers(1)	//1: The list of object states
		.WithSamplers(1)		//2: The scene shadow map
		.WithSamplers(1)		//3: The scene cubemap

		.Build("Global Data Layout");

	objectTextxureLayout = DescriptorSetLayoutBuilder(GetDevice())
		.WithSamplers(_TEXCOUNT)		//0: Object textures live here
		.Build("Object Texture Layout");

	objectTextxureDescriptor = BuildUniqueDescriptorSet(*objectTextxureLayout);

	defaultTexture = TextureBuilder(GetDevice(), GetMemoryAllocator())
		.WithPool(GetCommandPool(CommandBuffer::Graphics))
		.WithQueue(GetQueue(CommandBuffer::Graphics))
		.BuildFromFile("Default.png");

	loadedTextures.push_back(&(*defaultTexture)); //Default will always be index 0!

	defaultSampler = GetDevice().createSamplerUnique(
		vk::SamplerCreateInfo()
		.setAnisotropyEnable(false)
		.setMaxAnisotropy(16)
		.setMinFilter(vk::Filter::eLinear)
		.setMagFilter(vk::Filter::eLinear)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMaxLod(80.0f)
	);
	for (int i = 0; i < _TEXCOUNT; ++i) {
		WriteImageDescriptor(*objectTextxureDescriptor, 0, i, defaultTexture->GetDefaultView(), *defaultSampler);
	}

	textSampler = GetDevice().createSamplerUnique(
		vk::SamplerCreateInfo()
		.setAnisotropyEnable(false)
		.setMaxAnisotropy(1)
		.setMinFilter(vk::Filter::eNearest)
		.setMagFilter(vk::Filter::eNearest)
		.setMipmapMode(vk::SamplerMipmapMode::eNearest)
		.setMaxLod(0.0f)
	);

	{//Setting up the data for the skybox
		quadMesh = GenerateQuad();

		cubeTex = TextureBuilder(GetDevice(), GetMemoryAllocator())
			.WithPool(GetCommandPool(CommandBuffer::Graphics))
			.WithQueue(GetQueue(CommandBuffer::Graphics))
			.BuildCubemapFromFile(
				"Cubemap/skyrender0004.png", "Cubemap/skyrender0001.png",
				"Cubemap/skyrender0003.png", "Cubemap/skyrender0006.png",
				"Cubemap/skyrender0002.png", "Cubemap/skyrender0005.png",
				"Cubemap Texture!"
			);

		skyboxShader = ShaderBuilder(GetDevice())
			.WithVertexBinary("skybox.vert.spv")
			.WithFragmentBinary("skybox.frag.spv")
			.Build("Basic Shader!");

		skyboxPipeline = PipelineBuilder(GetDevice())
			.WithVertexInputState(quadMesh->GetVertexInputState())
			.WithTopology(quadMesh->GetVulkanTopology())
			.WithShader(skyboxShader)
			.WithColourAttachment(GetSurfaceFormat())
			.WithDepthAttachment(depthBuffer->GetFormat(), vk::CompareOp::eAlways, true, false)
			.WithDescriptorSetLayout(0, *globalDataLayout) //Set 0
			.Build("CubeMapRenderer Skybox Pipeline");
	}

	{//Setting up all the data we need for shadow maps!
		shadowShader = ShaderBuilder(GetDevice())
			.WithVertexBinary("Shadow.vert.spv")
			.WithFragmentBinary("Shadow.frag.spv")
			.Build("Shadow Fill Shader");

		shadowMap = TextureBuilder(GetDevice(), GetMemoryAllocator())
			.WithPool(GetCommandPool(CommandBuffer::Graphics))
			.WithQueue(GetQueue(CommandBuffer::Graphics))
			.WithDimension(_SHADOWSIZE, _SHADOWSIZE)
			.WithAspects(vk::ImageAspectFlagBits::eDepth)
			.WithFormat(vk::Format::eD32Sfloat)
			.WithLayout(vk::ImageLayout::eDepthAttachmentOptimal)
			.WithUsages(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
			.WithMips(false)
			.Build("Shadowmap");

		shadowViewport = vk::Viewport(0.0f, (float)_SHADOWSIZE, (float)_SHADOWSIZE, -(float)_SHADOWSIZE, 0.0f, 1.0f);
		shadowScissor = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(_SHADOWSIZE, _SHADOWSIZE));
	}
	{//
		sceneShader = ShaderBuilder(GetDevice())
			.WithVertexBinary("Scene.vert.spv")
			.WithFragmentBinary("Scene.frag.spv")
			.Build("Scene Shader");
	}
	{//Data structures used by the debug drawing
		BuildDebugPipelines();
	}
	//We're going to have 3 frames in-flight at once, with their own per-frame data

	allFrames = new FrameData[_FRAMECOUNT];
	for (int i = 0; i < _FRAMECOUNT; ++i) {
		allFrames[i].fence = GetDevice().createFenceUnique({});

		{//We store scene object matrices in a big UBO
			allFrames[i].debugVertSize = 10000;
			allFrames[i].objectCount = 1024;

			allFrames[i].dataBuffer = BufferBuilder(GetDevice(), GetMemoryAllocator())
				.WithBufferUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eVertexBuffer)
				.WithHostVisibility()
				.Build(1024 * 1024 * 64, "Lights");

			allFrames[i].data = allFrames[i].dataBuffer.Map<char>();

			//	CreateBuffer(1024 * 1024 * 64, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible);
			//allFrames[i].data = (char*)device.mapMemory(*allFrames[i].dataBuffer.deviceMem, 0, allFrames[i].dataBuffer.allocInfo.allocationSize);
			allFrames[i].dataStart = allFrames[i].data;

			allFrames[i].dataDescriptor = BuildUniqueDescriptorSet(*globalDataLayout);

			WriteImageDescriptor(*allFrames[i].dataDescriptor, 2, 0, shadowMap->GetDefaultView(), *defaultSampler, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
			WriteImageDescriptor(*allFrames[i].dataDescriptor, 3, 0, cubeTex->GetDefaultView(), *defaultSampler);
		}
	}
	currentFrameIndex = 0;
	currentFrame = &allFrames[currentFrameIndex];

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadTexture("PressStart2P.png"));
}


//Defer making the scene pipelines until a mesh has been loaded
//We're assuming that all loaded meshes have the same vertex format...
void GameTechVulkanRenderer::BuildScenePipelines(VulkanMesh* m) {
	scenePipeline = PipelineBuilder(GetDevice())
		.WithVertexInputState(m->GetVertexInputState())
		.WithTopology(vk::PrimitiveTopology::eTriangleList)
		.WithShader(sceneShader)		
		.WithColourAttachment(GetSurfaceFormat(), vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha)
		.WithDepthAttachment(depthBuffer->GetFormat(), vk::CompareOp::eLessOrEqual, true, true)
		.WithDescriptorSetLayout(0, *globalDataLayout)
		.WithDescriptorSetLayout(1, *objectTextxureLayout) //Set 1
		.WithPushConstant(vk::ShaderStageFlagBits::eVertex, 0, sizeof(Matrix4))
		.Build("Main Scene Pipeline");

	shadowPipeline = PipelineBuilder(GetDevice())
		.WithVertexInputState(m->GetVertexInputState())
		.WithTopology(vk::PrimitiveTopology::eTriangleList)
		.WithShader(shadowShader)
		.WithPushConstant(vk::ShaderStageFlagBits::eVertex, 0, sizeof(Matrix4))
		.WithDescriptorSetLayout(0, *globalDataLayout)
		.WithDescriptorSetLayout(1, *objectTextxureLayout) //Set 1
		.WithDepthAttachment(shadowMap->GetFormat(), vk::CompareOp::eLessOrEqual, true, true)
		.WithRaster(vk::CullModeFlagBits::eFront)
		.Build("Shadow Map Pipeline");
}

void GameTechVulkanRenderer::BuildDebugPipelines() {
	debugLineShader = ShaderBuilder(GetDevice())
		.WithVertexBinary("Debug.vert.spv")
		.WithFragmentBinary("Debug.frag.spv")
		.Build("Debug Line Shader");

	debugTextShader = ShaderBuilder(GetDevice())
		.WithVertexBinary("DebugText.vert.spv")
		.WithFragmentBinary("DebugText.frag.spv")
		.Build("Debug Text Shader");

	vk::VertexInputAttributeDescription debugLineVertAttribs[2] = {
		//Shader input, binding, format, offset
		{0,0,vk::Format::eR32G32B32A32Sfloat, 0},
		{1,1,vk::Format::eR32G32B32A32Sfloat, 0},
	};

	vk::VertexInputBindingDescription debugLineVertBindings[2] = {
		//Binding, stride, input rate
		{0, lineStride, vk::VertexInputRate::eVertex},//Positions
		{1, lineStride, vk::VertexInputRate::eVertex},//Colours
	};

	vk::VertexInputAttributeDescription debugTextVertAttribs[3] = {
		//Shader input, binding, format, offset
		{0,0,vk::Format::eR32G32Sfloat		, 0},
		{1,1,vk::Format::eR32G32Sfloat		, 0},
		{2,2,vk::Format::eR32G32B32A32Sfloat, 0},
	};

	vk::VertexInputBindingDescription debugTextVertBindings[3] = {
		//Binding, stride, input rate
		{0, textStride, vk::VertexInputRate::eVertex},//Positions
		{1, textStride, vk::VertexInputRate::eVertex},//Tex coords
		{2, textStride, vk::VertexInputRate::eVertex},//Colours
	};

	vk::PipelineVertexInputStateCreateInfo lineVertexState({}, debugLineVertBindings, debugLineVertAttribs);
	vk::PipelineVertexInputStateCreateInfo textVertexState({}, debugTextVertBindings, debugTextVertAttribs);

	debugLinePipeline = PipelineBuilder(GetDevice())
		.WithVertexInputState(lineVertexState)
		.WithTopology(vk::PrimitiveTopology::eLineList)
		.WithShader(debugLineShader)
		.WithColourAttachment(GetSurfaceFormat())
		.WithDepthAttachment(depthBuffer->GetFormat())
		.WithDescriptorSetLayout(0, *globalDataLayout)		//Set 0
		.WithDescriptorSetLayout(1, *objectTextxureLayout) //Set 1
		.Build("Debug Line Pipeline");

	debugTextPipeline = PipelineBuilder(GetDevice())
		.WithVertexInputState(textVertexState)
		.WithTopology(vk::PrimitiveTopology::eTriangleList)
		.WithShader(debugTextShader)
		.WithColourAttachment(GetSurfaceFormat())
		.WithDepthAttachment(depthBuffer->GetFormat())
		.WithDescriptorSetLayout(0, *globalDataLayout)		//Set 0
		.WithDescriptorSetLayout(1, *objectTextxureLayout) //Set 1
		.WithPushConstant(vk::ShaderStageFlagBits::eFragment, 0, sizeof(int)) //TexID to use
		.Build("Debug Text Pipeline");
}

void GameTechVulkanRenderer::RenderFrame() {
	TransitionUndefinedToColour(frameCmds, GetCurrentSwapImage());

	int texID = _TEXCOUNT - 1;
	VulkanTexture* tex = (VulkanTexture*)Debug::GetDebugFont()->GetTexture();
	WriteImageDescriptor(*objectTextxureDescriptor, 0, texID, tex->GetDefaultView(), *textSampler);

	GlobalData frameData;
	frameData.lightColour	= Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	frameData.lightRadius	= 1000.0f;
	frameData.lightPosition = Vector3(-100.0f, 60.0f, -100.0f);
	frameData.cameraPos		= gameWorld.GetMainCamera().GetPosition();

	frameData.viewMatrix	= gameWorld.GetMainCamera().BuildViewMatrix();
	frameData.projMatrix	= gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());
	frameData.orthoMatrix	= Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);
	frameData.shadowMatrix  =	Matrix4::Perspective(50.0f, 500.0f, 1, 45.0f) * 
								Matrix4::BuildViewMatrix(frameData.lightPosition, Vector3(0, 0, 0), Vector3(0, 1, 0));

	currentFrame->data				= currentFrame->dataStart;
	currentFrame->bytesWritten		= 0;
	currentFrame->globalDataOffset	= 0;
	currentFrame->objectStateOffset = sizeof(GlobalData);
	currentFrame->debugLinesOffset	= currentFrame->objectStateOffset;

	currentFrame->WriteData<GlobalData>(frameData); //Let's start filling up our frame data!

	currentFrame->AlignData(128);
	currentFrame->objectStateOffset = currentFrame->bytesWritten;
	
	UpdateObjectList();

	currentFrame->AlignData(128);

	size_t objectSize = currentFrame->bytesWritten - currentFrame->objectStateOffset;

	WriteBufferDescriptor(*currentFrame->dataDescriptor, 0, vk::DescriptorType::eUniformBuffer, currentFrame->dataBuffer, 0, sizeof(GlobalData));
	WriteBufferDescriptor(*currentFrame->dataDescriptor, 1, vk::DescriptorType::eStorageBuffer, currentFrame->dataBuffer, currentFrame->objectStateOffset, objectSize);

	{//Render the shadow map for the frame	
		DynamicRenderBuilder()
			.WithDepthAttachment(shadowMap->GetDefaultView())
			.WithRenderArea(shadowScissor)
			.BeginRendering(frameCmds);

		frameCmds.setViewport(0, 1, &shadowViewport);
		frameCmds.setScissor(0, 1, &shadowScissor);

		RenderSceneObjects(shadowPipeline, frameCmds); //Draw the shadow map

		frameCmds.endRendering();
	}
	
	//Now render the main scene view
	TransitionDepthToSampler(frameCmds, *shadowMap);

	frameCmds.setViewport(0, 1, &defaultViewport);
	frameCmds.setScissor(0, 1, &defaultScissor);

	DynamicRenderBuilder()
	.WithColourAttachment(GetCurrentSwapView())
	.WithDepthAttachment(depthBuffer->GetDefaultView())
	.WithRenderArea(defaultScreenRect)
	.BeginRendering(frameCmds);

	RenderSkybox(frameCmds);
	RenderSceneObjects(scenePipeline, frameCmds); //Draw the main scene

	UpdateDebugData();
	RenderDebugLines(frameCmds);
	RenderDebugText(frameCmds);

	frameCmds.endRendering();

	TransitionSamplerToDepth(frameCmds, *shadowMap);

	currentFrameIndex = (currentFrameIndex + 1) % _FRAMECOUNT;
	currentFrame = &allFrames[currentFrameIndex];
}

void GameTechVulkanRenderer::UpdateObjectList() {
	activeObjects.clear();

	int objectCount = 0;

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				objectCount++;
			}
		}
	);

	VulkanMesh* pipeMesh = nullptr;
	int at = 0;
	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				RenderObject* g = o->GetRenderObject();
				if (g) {
					activeObjects.emplace_back(g);

					ObjectState state;
					state.modelMatrix = g->GetTransform()->GetMatrix();
					state.colour = g->GetColour();
					state.index[0] = 0;
					if (g->GetMesh()) {
						pipeMesh = (VulkanMesh*)g->GetMesh();
					}
					if (g->GetDefaultTexture()) {
						VulkanTexture* t = (VulkanTexture*)g->GetDefaultTexture();
						state.index[0] = t->GetAssetID();
					}
					currentFrame->WriteData<ObjectState>(state);
					currentFrame->debugLinesOffset += sizeof(ObjectState);
					at++;
				}
			}
		}
	);
	if (pipeMesh && !scenePipeline.pipeline) {
		BuildScenePipelines(pipeMesh);
	}
}

void GameTechVulkanRenderer::UpdateDebugData() {
	const std::vector<Debug::DebugStringEntry>& strings = Debug::GetDebugStrings();
	const std::vector<Debug::DebugLineEntry>& lines		= Debug::GetDebugLines();

	currentFrame->textVertCount = 0;
	currentFrame->lineVertCount = 0;

	for (const auto& s : strings) {
		currentFrame->textVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	currentFrame->lineVertCount = (int)lines.size() * 2;

	currentFrame->lineVertSize = currentFrame->lineVertCount * lineStride;
	currentFrame->textVertSize = currentFrame->textVertCount * textStride;

	currentFrame->debugLinesOffset = currentFrame->bytesWritten;

	currentFrame->WriteData((void*)lines.data(), (size_t)currentFrame->lineVertCount * lineStride);

	currentFrame->debugTextOffset = currentFrame->bytesWritten;
	std::vector< NCL::Rendering::SimpleFont::InterleavedTextVertex> verts;

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildInterleavedVerticesForString(s.data, s.position, s.colour, size, verts);
		//can now copy to GPU visible mem
		size_t count = verts.size() * textStride;
		memcpy(currentFrame->data, verts.data(), count);
		currentFrame->data += count;
		currentFrame->bytesWritten += count;
		verts.clear();
	}
}

void GameTechVulkanRenderer::RenderSceneObjects(VulkanPipeline& pipe, vk::CommandBuffer cmds) {
	int index = 0;
	if (activeObjects.empty()) {
		return;
	}

	cmds.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipe.pipeline);
	cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipe.layout, 0, 1, &*currentFrame->dataDescriptor, 0, nullptr);
	cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipe.layout, 1, 1, &*objectTextxureDescriptor, 0, nullptr);

	VulkanMesh* activeMesh = (VulkanMesh*)activeObjects[0]->GetMesh();
	int count = 1;
	int startingIndex = 0;

	for (int i = 1; i < activeObjects.size(); ++i) {
		VulkanMesh* objectMesh = (VulkanMesh*)activeObjects[i]->GetMesh();
		int increment = 1;
		//The new mesh is different than previous meshes, flush out the old list
		if (activeMesh != objectMesh) {
			cmds.pushConstants(*pipe.layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(uint32_t), (void*)&startingIndex);
			DrawMesh(cmds, *activeMesh, count);
			activeMesh = objectMesh;
			count = 1;
			startingIndex = i;
			increment = 0;
		}
		if (i == activeObjects.size()-1) {
			cmds.pushConstants(*pipe.layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(uint32_t), (void*)&startingIndex);
			DrawMesh(cmds, *objectMesh, count);
		}
		else {
			count += increment;
		}
	}
}

void GameTechVulkanRenderer::RenderSkybox(vk::CommandBuffer cmds) {
	//First bit: Draw skybox!
	cmds.bindPipeline(vk::PipelineBindPoint::eGraphics, *skyboxPipeline.pipeline);
	cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *skyboxPipeline.layout, 0, 1, &*currentFrame->dataDescriptor, 0, nullptr);
	DrawMesh(cmds, *quadMesh);
}

void GameTechVulkanRenderer::RenderDebugLines(vk::CommandBuffer cmds) {
	cmds.bindPipeline(vk::PipelineBindPoint::eGraphics, *debugLinePipeline.pipeline);
	cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *debugLinePipeline.layout, 0, 1, &*currentFrame->dataDescriptor, 0, nullptr);
	cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *debugLinePipeline.layout, 1, 1, &*objectTextxureDescriptor, 0, nullptr);

	vk::Buffer attributeBuffers[2] = {
		currentFrame->dataBuffer.buffer,
		currentFrame->dataBuffer.buffer
	};

	vk::DeviceSize attributeOffsets[2] = {
		currentFrame->debugLinesOffset + 0,
		currentFrame->debugLinesOffset + sizeof(Vector4)
	};

	cmds.bindVertexBuffers(0, 2, attributeBuffers, attributeOffsets);
	cmds.draw((uint32_t)currentFrame->lineVertCount, 1, 0, 0);
}

void GameTechVulkanRenderer::RenderDebugText(vk::CommandBuffer cmds) {
	cmds.bindPipeline(vk::PipelineBindPoint::eGraphics, *debugTextPipeline.pipeline);
	cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *debugTextPipeline.layout, 0, 1, &*currentFrame->dataDescriptor, 0, nullptr);
	cmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *debugTextPipeline.layout, 1, 1, &*objectTextxureDescriptor, 0, nullptr);

	int texID = _TEXCOUNT-1;
	cmds.pushConstants(*debugTextPipeline.layout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(uint32_t), (void*)&texID);

	vk::Buffer attributeBuffers[3] = {
		currentFrame->dataBuffer.buffer,
		currentFrame->dataBuffer.buffer,
		currentFrame->dataBuffer.buffer
	};

	vk::DeviceSize attributeOffsets[3] = {
		currentFrame->debugTextOffset + 0,
		currentFrame->debugTextOffset + sizeof(Vector2), //jump over the position
		currentFrame->debugTextOffset + sizeof(Vector2) + sizeof(Vector2) //jump over position and tex coord
	};

	cmds.bindVertexBuffers(0, 3, attributeBuffers, attributeOffsets);
	cmds.draw((uint32_t)currentFrame->textVertCount, 1, 0, 0);
}

Mesh* GameTechVulkanRenderer::LoadMesh(const string& name) {
	VulkanMesh* newMesh = new VulkanMesh();

	MshLoader::LoadMesh(name, *newMesh);

	newMesh->SetPrimitiveType(NCL::GeometryPrimitive::Triangles);
	newMesh->SetDebugName(name);
	newMesh->UploadToGPU(this);
	return newMesh;
}

Texture* GameTechVulkanRenderer::LoadTexture(const string& name) {
	VulkanTexture* t = TextureBuilder(GetDevice(), GetMemoryAllocator())
		.WithPool(GetCommandPool(CommandBuffer::Graphics))
		.WithQueue(GetQueue(CommandBuffer::Graphics))
		.BuildFromFile(name).release();

	//Write the texture to our big descriptor set
	WriteImageDescriptor(*objectTextxureDescriptor, 0, (int)loadedTextures.size(), t->GetDefaultView(), *defaultSampler);
	t->SetAssetID(loadedTextures.size());
	loadedTextures.push_back(t);
	return t;
}

Shader* GameTechVulkanRenderer::LoadShader(const string& vertex, const string& fragment) {
	return ShaderBuilder(GetDevice())
		.WithVertexBinary(vertex + ".spv")
		.WithFragmentBinary(fragment + ".spv")
		.Build("Shader!").release();
	return nullptr;
}

UniqueVulkanMesh GameTechVulkanRenderer::GenerateQuad() {
	VulkanMesh* quadMesh = new VulkanMesh();
	quadMesh->SetVertexPositions({ Vector3(-1,-1,0), Vector3(1,-1,0), Vector3(1,1,0), Vector3(-1,1,0) });
	quadMesh->SetVertexTextureCoords({ Vector2(0,0), Vector2(1,0), Vector2(1, 1), Vector2(0, 1) });
	quadMesh->SetVertexIndices({ 0,1,3,2 });
	quadMesh->SetDebugName("Fullscreen Quad");
	quadMesh->SetPrimitiveType(NCL::GeometryPrimitive::TriangleStrip);
	quadMesh->UploadToGPU(this);

	return UniqueVulkanMesh(quadMesh);
}
#endif
