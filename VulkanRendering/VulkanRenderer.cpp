/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanRenderer.h"
#include "VulkanMesh.h"
#include "VulkanTexture.h"
#include "VulkanTextureBuilder.h"

#include "VulkanUtils.h"

#ifdef _WIN32
#include "Win32Window.h"
using namespace NCL::Win32Code;
#endif

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

vk::PhysicalDeviceDescriptorIndexingFeatures indexingFeatures;

VulkanRenderer::VulkanRenderer(Window& window) : RendererBase(window) {
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = Vulkan::dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	allocatorInfo		= {};

	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	deviceExtensions.push_back("VK_KHR_dynamic_rendering");		//Now in core 1.3
	deviceExtensions.push_back("VK_KHR_maintenance4");			//Now in core 1.3
	deviceExtensions.push_back("VK_KHR_depth_stencil_resolve");	//Now in core 1.2
	deviceExtensions.push_back("VK_KHR_create_renderpass2");		//Now in core 1.2

	instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#ifdef WIN32
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
	
	deviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");

	instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
}

VulkanRenderer::~VulkanRenderer() {
	device.waitIdle();
	depthBuffer.reset();

	for (auto& i : swapChainList) {
		device.destroyImageView(i->view);
	};

	for (unsigned int i = 0; i < numFrameBuffers; ++i) {
		device.destroyFramebuffer(frameBuffers[i]);
	}
	vmaDestroyAllocator(memoryAllocator);
	device.destroyDescriptorPool(defaultDescriptorPool);
	device.destroySwapchainKHR(swapChain);

	device.destroyCommandPool(commandPools[CommandBuffer::Graphics]);
	device.destroyCommandPool(commandPools[CommandBuffer::Copy]);
	device.destroyCommandPool(commandPools[CommandBuffer::AsyncCompute]);

	device.destroyRenderPass(defaultRenderPass);
	device.destroyPipelineCache(pipelineCache);
	device.destroy(); //Destroy everything except instance before this gets destroyed!

	instance.destroySurfaceKHR(surface);
	instance.destroy();

	delete[] frameBuffers;
}

bool VulkanRenderer::Init() {
	InitInstance();

	InitPhysicalDevice();

	InitGPUDevice();
	InitMemoryAllocator();

	InitCommandPools();
	InitDefaultDescriptorPool();

	hostWindow.SetRenderer(this);
	//OnWindowResize(hostWindow.GetScreenSize().x, hostWindow.GetScreenSize().y);

	pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());

	frameCmds = swapChainList[currentSwap]->frameCmds;

	return true;
}

bool VulkanRenderer::InitInstance() {
	vk::ApplicationInfo appInfo = vk::ApplicationInfo(this->hostWindow.GetTitle().c_str());

	appInfo.apiVersion = VK_MAKE_VERSION(majorVersion, minorVersion, 0);

	vk::InstanceCreateInfo instanceInfo = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &appInfo)
		.setEnabledExtensionCount((uint32_t)instanceExtensions.size())
		.setPpEnabledExtensionNames(instanceExtensions.data())
		.setEnabledLayerCount((uint32_t)instanceLayers.size())
		.setPpEnabledLayerNames(instanceLayers.data());

	instance = vk::createInstance(instanceInfo);

	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

	return true;
}

bool	VulkanRenderer::InitPhysicalDevice() {
	auto enumResult = instance.enumeratePhysicalDevices();

	if (enumResult.empty()) {
		return false; //Guess there's no Vulkan capable devices?!
	}

	gpu = enumResult[0];
	for (auto& i : enumResult) {
		if (i.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
			gpu = i; //Prefer a discrete GPU on multi device machines like laptops
		}
	}

	std::cout << __FUNCTION__ << " Vulkan using physical device " << gpu.getProperties().deviceName << "\n";

	return true;
}

bool VulkanRenderer::InitGPUDevice() {
	InitSurface();
	InitDeviceQueueIndices();

	float queuePriority = 0.0f;

	std::vector< vk::DeviceQueueCreateInfo> queueInfos;

	queueInfos.emplace_back(vk::DeviceQueueCreateInfo()
		.setQueueCount(1)
		.setQueueFamilyIndex(gfxQueueIndex)
		.setPQueuePriorities(&queuePriority));

	if (computeQueueIndex != gfxQueueIndex){
		queueInfos.emplace_back(vk::DeviceQueueCreateInfo()
			.setQueueCount(1)
			.setQueueFamilyIndex(computeQueueIndex)
			.setPQueuePriorities(&queuePriority));
	}
	if (copyQueueIndex != gfxQueueIndex) {
		queueInfos.emplace_back(vk::DeviceQueueCreateInfo()
			.setQueueCount(1)
			.setQueueFamilyIndex(copyQueueIndex)
			.setPQueuePriorities(&queuePriority));
	}

	vk::PhysicalDeviceFeatures features = vk::PhysicalDeviceFeatures()
		.setMultiDrawIndirect(true)
		.setDrawIndirectFirstInstance(true)
		.setShaderClipDistance(true)
		.setShaderCullDistance(true);

	vk::PhysicalDeviceRobustness2FeaturesEXT robustness;

	vk::PhysicalDeviceFeatures2 deviceFeatures;
	deviceFeatures.pNext = (void*)&robustness;

	gpu.getFeatures2(&deviceFeatures);

	vk::DeviceCreateInfo createInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(queueInfos.size())
		.setPQueueCreateInfos(queueInfos.data());

	SetupDevice(deviceFeatures);	

	vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering(true);
	dynamicRendering.pNext = deviceFeatures.pNext;
	deviceFeatures.pNext = &dynamicRendering;
	
	createInfo.setEnabledLayerCount((uint32_t)deviceLayers.size())
		.setPpEnabledLayerNames(deviceLayers.data())
		.setEnabledExtensionCount((uint32_t)deviceExtensions.size())
		.setPpEnabledExtensionNames(deviceExtensions.data());

	createInfo.pNext = &deviceFeatures;

	device = gpu.createDevice(createInfo);

	queueTypes[CommandBuffer::Graphics]		= device.getQueue(gfxQueueIndex, 0);
	queueTypes[CommandBuffer::AsyncCompute]	= device.getQueue(computeQueueIndex, 0);
	queueTypes[CommandBuffer::Copy]			= device.getQueue(copyQueueIndex, 0);
	presentQueue	= device.getQueue(gfxPresentIndex, 0);

	deviceMemoryProperties = gpu.getMemoryProperties();
	deviceProperties = gpu.getProperties();

	VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
	return true;
}

bool VulkanRenderer::InitSurface() {
#ifdef _WIN32
	Win32Window* window = (Win32Window*)&hostWindow;

	vk::Win32SurfaceCreateInfoKHR createInfo;

	createInfo = vk::Win32SurfaceCreateInfoKHR(
		vk::Win32SurfaceCreateFlagsKHR(), window->GetInstance(), window->GetHandle());

	surface = instance.createWin32SurfaceKHR(createInfo);
#endif

	auto formats = gpu.getSurfaceFormatsKHR(surface);

	if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
		surfaceFormat	= vk::Format::eB8G8R8A8Unorm;
		surfaceSpace	= formats[0].colorSpace;
	}
	else {
		surfaceFormat	= formats[0].format;
		surfaceSpace	= formats[0].colorSpace;
	}

	return formats.size() > 0;
}

uint32_t VulkanRenderer::InitBufferChain(vk::CommandBuffer  cmdBuffer) {
	vk::SwapchainKHR oldChain					= swapChain;
	std::vector<SwapChain*> oldSwapChainList	= swapChainList;
	swapChainList.clear();

	vk::SurfaceCapabilitiesKHR surfaceCaps = gpu.getSurfaceCapabilitiesKHR(surface);

	vk::Extent2D swapExtents = vk::Extent2D((int)hostWindow.GetScreenSize().x, (int)hostWindow.GetScreenSize().y);

	auto presentModes = gpu.getSurfacePresentModesKHR(surface); //Type is of vector of PresentModeKHR

	vk::PresentModeKHR idealPresentMode = vk::PresentModeKHR::eFifo;

	for (const auto& i : presentModes) {
		if (i == vk::PresentModeKHR::eMailbox) {
			idealPresentMode = i;
			break;
		}
		else if (i == vk::PresentModeKHR::eImmediate) {
			idealPresentMode = vk::PresentModeKHR::eImmediate; //Might still become mailbox...
		}
	}

	vk::SurfaceTransformFlagBitsKHR idealTransform;

	if (surfaceCaps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
		idealTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else {
		idealTransform = surfaceCaps.currentTransform;
	}

	int idealImageCount = surfaceCaps.minImageCount + 1;
	if (surfaceCaps.maxImageCount > 0) {
		idealImageCount = std::min(idealImageCount, (int)surfaceCaps.maxImageCount);
	}

	vk::SwapchainCreateInfoKHR swapInfo;

	swapInfo.setPresentMode(idealPresentMode)
		.setPreTransform(idealTransform)
		.setSurface(surface)
		.setImageColorSpace(surfaceSpace)
		.setImageFormat(surfaceFormat)
		.setImageExtent(swapExtents)
		.setMinImageCount(idealImageCount)
		.setOldSwapchain(oldChain)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	swapChain = device.createSwapchainKHR(swapInfo);

	if (!oldSwapChainList.empty()) {
		for (unsigned int i = 0; i < numFrameBuffers; ++i) {
			device.destroyImageView(oldSwapChainList[i]->view);
			delete oldSwapChainList[i];
		}
	}
	if (oldChain) {
		device.destroySwapchainKHR(oldChain);
	}

	auto images = device.getSwapchainImagesKHR(swapChain);

	for (auto& i : images) {
		vk::ImageViewCreateInfo viewCreate = vk::ImageViewCreateInfo()
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setFormat(surfaceFormat)
			.setImage(i)
			.setViewType(vk::ImageViewType::e2D);

		SwapChain* chain = new SwapChain();

		chain->image = i;

		ImageTransitionBarrier(cmdBuffer, i, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput);

		chain->view = device.createImageView(viewCreate);

		swapChainList.push_back(chain);

		auto buffers = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(
			commandPools[CommandBuffer::Graphics], vk::CommandBufferLevel::ePrimary, 1));

		chain->frameCmds = buffers[0];
	}

	return (int)images.size();
}

void	VulkanRenderer::InitCommandPools() {	
	commandPools[CommandBuffer::Graphics] = device.createCommandPool(vk::CommandPoolCreateInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer, gfxQueueIndex));

	commandPools[CommandBuffer::AsyncCompute] = device.createCommandPool(vk::CommandPoolCreateInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer, computeQueueIndex));

	commandPools[CommandBuffer::Copy] = device.createCommandPool(vk::CommandPoolCreateInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer, copyQueueIndex));
}

void	VulkanRenderer::InitMemoryAllocator() {
	VmaVulkanFunctions funcs = {};
	funcs.vkGetInstanceProcAddr = ::vk::defaultDispatchLoaderDynamic.vkGetInstanceProcAddr;
	funcs.vkGetDeviceProcAddr   = ::vk::defaultDispatchLoaderDynamic.vkGetDeviceProcAddr;

	allocatorInfo.physicalDevice = gpu;
	allocatorInfo.device	= device;
	allocatorInfo.instance	= instance;

	allocatorInfo.pVulkanFunctions = &funcs;
	vmaCreateAllocator(&allocatorInfo, &memoryAllocator);
}

bool VulkanRenderer::InitDeviceQueueIndices() {
	deviceQueueProps = gpu.getQueueFamilyProperties();

	VkBool32 supportsPresent = false;
	gfxQueueIndex		= -1;
	computeQueueIndex	= -1;
	copyQueueIndex		= -1;
	gfxPresentIndex		= -1;

	for (unsigned int i = 0; i < deviceQueueProps.size(); ++i) {
		supportsPresent = gpu.getSurfaceSupportKHR(i, surface);

		if (gfxQueueIndex == -1 && deviceQueueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			gfxQueueIndex = i;
			if (supportsPresent && gfxPresentIndex == -1) {
				gfxPresentIndex = i;
			}
		}

		if (i != gfxQueueIndex && computeQueueIndex == -1 && deviceQueueProps[i].queueFlags & vk::QueueFlagBits::eCompute) {
			computeQueueIndex = i;
		}

		if (i != gfxQueueIndex && copyQueueIndex == -1 && deviceQueueProps[i].queueFlags & vk::QueueFlagBits::eTransfer) {
			copyQueueIndex = i;
		}
	}

	if (computeQueueIndex == -1) {
		computeQueueIndex = gfxPresentIndex;
	}
	else {
		std::cout << __FUNCTION__ << " Device supports async compute!\n";
	}

	if (copyQueueIndex == -1) {
		copyQueueIndex = gfxPresentIndex;
	}
	else {
		std::cout << __FUNCTION__ << " Device supports async copy!\n";
	}

	if (gfxQueueIndex == -1		|| 
		gfxPresentIndex == -1   || 
		computeQueueIndex == -1 ||
		copyQueueIndex == -1) {
		return false;
	}

	return true;
}

void VulkanRenderer::OnWindowResize(int width, int height) {
	if (!hostWindow.IsMinimised() && width == windowSize.x && height == windowSize.y) {
		return;
	}
	if (width == 0 && height == 0) {
		return;
	}
	windowSize = { width, height };

	defaultScreenRect = vk::Rect2D({ 0,0 }, { (uint32_t)windowSize.x, (uint32_t)windowSize.y });

	defaultViewport = vk::Viewport(0.0f, (float)windowSize.y, (float)windowSize.x, -(float)windowSize.y, 0.0f, 1.0f);
	defaultScissor	= vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(windowSize.x, windowSize.y));

	defaultClearValues[0] = vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{0.2f, 0.2f, 0.2f, 1.0f}));
	defaultClearValues[1] = vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0));

	vk::UniqueCommandBuffer cmds = CmdBufferBegin(device, commandPools[CommandBuffer::Graphics], "Window resize cmds");

	std::cout << __FUNCTION__ << " New dimensions: " << windowSize.x << " , " << windowSize.y << "\n";

	device.waitIdle();

	depthBuffer = TextureBuilder(GetDevice(), GetMemoryAllocator())
		.WithPool(GetCommandPool(CommandBuffer::Graphics))
		.WithQueue(GetQueue(CommandBuffer::Graphics))
		.WithDimension(hostWindow.GetScreenSize().x, hostWindow.GetScreenSize().y)
		.WithAspects(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil)
		.WithFormat(vk::Format::eD24UnormS8Uint)
		.WithLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.WithUsages(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
		.WithPipeFlags(vk::PipelineStageFlagBits::eEarlyFragmentTests)
		.WithMips(false)
		.Build("Depth Buffer");

	//depthBuffer = VulkanTexture::CreateDepthTexture(this,hostWindow.GetScreenSize().x, hostWindow.GetScreenSize().y);
	
	numFrameBuffers = InitBufferChain(*cmds);

	InitDefaultRenderPass();
	CreateDefaultFrameBuffers();

	device.waitIdle();

	vk::Semaphore	presentSempaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
	vk::Fence		fence = device.createFence(vk::FenceCreateInfo());

	currentSwap = device.acquireNextImageKHR(swapChain, UINT64_MAX, presentSempaphore, vk::Fence()).value;	//Get swap image

	device.destroySemaphore(presentSempaphore);
	device.destroyFence(fence);

	CompleteResize();
	CmdBufferEndSubmitWait(*cmds, device, queueTypes[CommandBuffer::Graphics]);
}

void VulkanRenderer::CompleteResize() {

}

void	VulkanRenderer::BeginFrame() {
	frameCmds.reset({});

	frameCmds.begin(vk::CommandBufferBeginInfo());
	frameCmds.setViewport(0, 1, &defaultViewport);
	frameCmds.setScissor(0, 1, &defaultScissor);

	if (autoTransitionFrameBuffer) {
		TransitionUndefinedToColour(frameCmds, swapChainList[currentSwap]->image);
	}
	if (autoBeginDynamicRendering) {
		BeginDefaultRendering(frameCmds);
	}
}

void	VulkanRenderer::EndFrame() {
	if (autoBeginDynamicRendering) {
		frameCmds.endRendering();
	}

	if (hostWindow.IsMinimised()) {
		CmdBufferEndSubmitWait(frameCmds, device, queueTypes[CommandBuffer::Graphics]);
	}
	else {
		CmdBufferEndSubmit(frameCmds, queueTypes[CommandBuffer::Graphics]);
	}
}

void VulkanRenderer::SwapBuffers() {
	vk::UniqueSemaphore	presentSempaphore;
	vk::UniqueFence		presentFence;

	if (!hostWindow.IsMinimised()) {
		vk::CommandPool gfxPool		= commandPools[CommandBuffer::Graphics];
		vk::Queue		gfxQueue	= queueTypes[CommandBuffer::Graphics];

		vk::UniqueCommandBuffer cmds = CmdBufferBegin(device, gfxPool, "Window swap cmds");

		TransitionColourToPresent(*cmds, swapChainList[currentSwap]->image);
		CmdBufferEndSubmitWait(*cmds, device, gfxQueue);

		vk::Result presentResult = gfxQueue.presentKHR(vk::PresentInfoKHR(0, nullptr, 1, &swapChain, &currentSwap, nullptr));

		presentSempaphore = device.createSemaphoreUnique(vk::SemaphoreCreateInfo());
		presentFence	  = device.createFenceUnique(vk::FenceCreateInfo());
	
		currentSwap = device.acquireNextImageKHR(swapChain, UINT64_MAX, *presentSempaphore, *presentFence).value;	//Get swap image
	}
	defaultBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(defaultRenderPass)
		.setFramebuffer(frameBuffers[currentSwap])
		.setRenderArea(defaultScissor)
		.setClearValueCount(sizeof(defaultClearValues) / sizeof(vk::ClearValue))
		.setPClearValues(defaultClearValues);

	frameCmds = swapChainList[currentSwap]->frameCmds;

	if (!hostWindow.IsMinimised()) {
		vk::Result waitResult = device.waitForFences(*presentFence, true, ~0);
	}
}

void	VulkanRenderer::InitDefaultRenderPass() {
	if (defaultRenderPass) {
		device.destroyRenderPass(defaultRenderPass);
	}
	vk::AttachmentDescription attachments[] = {
		vk::AttachmentDescription()
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFormat(surfaceFormat)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
	,
		vk::AttachmentDescription()
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setFormat(depthBuffer->GetFormat())
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
	};

	vk::AttachmentReference references[] = {
		vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
		vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal)
	};

	vk::SubpassDescription subPass = vk::SubpassDescription()
		.setColorAttachmentCount(1)
		.setPColorAttachments(&references[0])
		.setPDepthStencilAttachment(&references[1])
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

	vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subPass);

	defaultRenderPass = device.createRenderPass(renderPassInfo);
}

bool VulkanRenderer::CreateDefaultFrameBuffers() {
	if (frameBuffers) {
		for (unsigned int i = 0; i < numFrameBuffers; ++i) {
			device.destroyFramebuffer(frameBuffers[i]);
		}
	}
	else {
		frameBuffers = new vk::Framebuffer[numFrameBuffers];
	}

	vk::ImageView attachments[2];
	
	vk::FramebufferCreateInfo createInfo = vk::FramebufferCreateInfo()
		.setWidth(hostWindow.GetScreenSize().x)
		.setHeight(hostWindow.GetScreenSize().y)
		.setLayers(1)
		.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setRenderPass(defaultRenderPass);

	for (uint32_t i = 0; i < numFrameBuffers; ++i) {
		attachments[0]	= swapChainList[i]->view;
		attachments[1]	= *depthBuffer->defaultView;
		frameBuffers[i] = device.createFramebuffer(createInfo);
	}

	defaultBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(defaultRenderPass)
		.setFramebuffer(frameBuffers[currentSwap])
		.setRenderArea(defaultScissor)
		.setClearValueCount(sizeof(defaultClearValues) / sizeof(vk::ClearValue))
		.setPClearValues(defaultClearValues);

	return true;
}

void	VulkanRenderer::InitDefaultDescriptorPool(uint32_t maxSets) {
	vk::DescriptorPoolSize poolSizes[] = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, maxSets),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, maxSets),
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, maxSets),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, maxSets),		
		
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, maxSets),
		vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, maxSets),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, maxSets),
#ifdef USE_RAY_TRACING
		vk::DescriptorPoolSize(vk::DescriptorType::eAccelerationStructureKHR, maxSets),
#endif
	};

	uint32_t poolCount = sizeof(poolSizes) / sizeof(vk::DescriptorPoolSize);

	vk::DescriptorPoolCreateInfo poolCreate;
	poolCreate.setPoolSizeCount(sizeof(poolSizes) / sizeof(vk::DescriptorPoolSize));
	poolCreate.setPPoolSizes(poolSizes);
	poolCreate.setMaxSets(maxSets * poolCount);
	poolCreate.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	defaultDescriptorPool = device.createDescriptorPool(poolCreate);
}

vk::UniqueDescriptorSet VulkanRenderer::BuildUniqueDescriptorSet(vk::DescriptorSetLayout  layout, vk::DescriptorPool pool, uint32_t variableDescriptorCount) {
	if (!pool) {
		pool = defaultDescriptorPool;
	}
	return Vulkan::BuildUniqueDescriptorSet(GetDevice(), layout, pool, variableDescriptorCount);
}

void	VulkanRenderer::WriteBufferDescriptor(vk::DescriptorSet set, int bindingSlot, vk::DescriptorType bufferType, vk::Buffer buff, size_t offset, size_t range) {
	Vulkan::WriteBufferDescriptor(GetDevice(), set, bindingSlot, bufferType, buff, offset, range);
}

void	VulkanRenderer::WriteImageDescriptor(vk::DescriptorSet set, int bindingSlot, int subIndex, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout) {
	Vulkan::WriteImageDescriptor(GetDevice(), set, bindingSlot, subIndex, view, sampler, layout);
}

void	VulkanRenderer::WriteStorageImageDescriptor(vk::DescriptorSet set, int bindingSlot, int subIndex, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout) {
	Vulkan::WriteStorageImageDescriptor(GetDevice(), set, bindingSlot, subIndex, view, sampler, layout);
}

void	VulkanRenderer::WriteTLASDescriptor(vk::DescriptorSet set, int bindingSlot, vk::AccelerationStructureKHR tlas) {
	Vulkan::WriteTLASDescriptor(GetDevice(), set, bindingSlot, tlas);
}

void VulkanRenderer::DrawMeshLayer(const VulkanMesh& m, unsigned int layer, vk::CommandBuffer  to, int instanceCount) {
	VkDeviceSize baseOffset = 0;

	const SubMesh* sm = m.GetSubMesh(layer);

	m.BindToCommandBuffer(to);

	if (m.GetIndexCount() > 0) {
		to.drawIndexed(sm->count, instanceCount, sm->start, sm->base, 0);
	}
	else {
		to.draw(sm->count, instanceCount, sm->start, 0);
	}
}

void VulkanRenderer::DrawMesh(vk::CommandBuffer  to, const VulkanMesh& m, int instanceCount) {
	VkDeviceSize baseOffset = 0;

	m.BindToCommandBuffer(to);

	if (m.GetIndexCount() > 0) {
		to.drawIndexed(m.GetIndexCount(), instanceCount, 0, 0, 0);
	}
	else {
		to.draw(m.GetVertexCount(), instanceCount, 0, 0);
	}
}

void	VulkanRenderer::BeginDefaultRenderPass(vk::CommandBuffer  cmds) {
	cmds.beginRenderPass(defaultBeginInfo, vk::SubpassContents::eInline);
	cmds.setViewport(0, 1, &defaultViewport);
	cmds.setScissor(0, 1, &defaultScissor);
}

void	VulkanRenderer::BeginDefaultRendering(vk::CommandBuffer  cmds) {
	vk::RenderingInfoKHR renderInfo;
	renderInfo.layerCount = 1;

	vk::RenderingAttachmentInfoKHR colourAttachment;
	colourAttachment.setImageView(swapChainList[currentSwap]->view)
		.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setClearValue(vk::ClearColorValue(0.2f, 0.2f, 0.2f, 1.0f));

	vk::RenderingAttachmentInfoKHR depthAttachment;
	depthAttachment.setImageView(depthBuffer->GetDefaultView())
		.setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.clearValue.setDepthStencil({ 1.0f, ~0U });

	renderInfo.setColorAttachments(colourAttachment)
		.setPDepthAttachment(&depthAttachment);
		//.setPStencilAttachment(&depthAttachment);

	renderInfo.setRenderArea(defaultScreenRect);

	cmds.beginRendering(renderInfo);
	cmds.setViewport(0, 1, &defaultViewport);
	cmds.setScissor(0, 1, &defaultScissor);
}

vk::Format VulkanRenderer::GetDepthFormat() const {
	return depthBuffer->GetFormat();
}