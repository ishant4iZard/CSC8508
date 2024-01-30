/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/RendererBase.h"
#include "../NCLCoreClasses/Maths.h"

#include "VulkanPipeline.h"
#include "SmartTypes.h"
#include "vma/vk_mem_alloc.h"
using std::string;

namespace NCL::Rendering::Vulkan {
	class VulkanMesh;
	class VulkanShader;
	class VulkanCompute;
	class VulkanTexture;
	struct VulkanBuffer;

	namespace CommandBuffer {
		enum Type : uint32_t {
			Graphics,
			AsyncCompute,
			Copy,
			MAX_BUFFERS
		};
	};

	class VulkanRenderer : public RendererBase {
		friend class VulkanMesh;
		friend class VulkanTexture;
	public:
		VulkanRenderer(Window& window);
		~VulkanRenderer();

		virtual bool Init();
		virtual bool HasInitialised() const { return device; }

	protected:
		void OnWindowResize(int w, int h)	override;
		void BeginFrame()		override;
		void EndFrame()			override;
		void SwapBuffers()		override;

		virtual void	CompleteResize();
		virtual void	InitDefaultRenderPass();
		virtual void	InitDefaultDescriptorPool(uint32_t maxSets = 128);

		void DrawMesh(vk::CommandBuffer  to, const VulkanMesh& m, int instanceCount = 1);
		void DrawMeshLayer(const VulkanMesh& m, unsigned int layer, vk::CommandBuffer  to, int instanceCount = 1);
	
		vk::UniqueDescriptorSet BuildUniqueDescriptorSet(vk::DescriptorSetLayout  layout, vk::DescriptorPool pool = {}, uint32_t variableDescriptorCount = 0);
		void	WriteBufferDescriptor(vk::DescriptorSet set, int bindingSlot, vk::DescriptorType bufferType, vk::Buffer buff, size_t offset = 0, size_t range = 0);
		void	WriteImageDescriptor(vk::DescriptorSet set, int bindingNum, int subIndex, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);
		void	WriteStorageImageDescriptor(vk::DescriptorSet set, int bindingNum, int subIndex, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);
		void	WriteTLASDescriptor(vk::DescriptorSet set, int bindingSlot, vk::AccelerationStructureKHR tlas);

		void	BeginDefaultRenderPass(vk::CommandBuffer cmds);
		void	BeginDefaultRendering(vk::CommandBuffer  cmds);

		vk::Device GetDevice() const {
			return device;
		}

		VmaAllocator GetMemoryAllocator() const {
			return memoryAllocator;
		}

		vk::Queue GetQueue(CommandBuffer::Type type) const {
			return queueTypes[type];
		}

		vk::CommandPool GetCommandPool(CommandBuffer::Type type) const {
			return commandPools[type];
		}

		vk::DescriptorPool GetDescriptorPool() {
			return defaultDescriptorPool;
		}

		vk::ImageView GetCurrentSwapView() const {
			return swapChainList[currentSwap]->view;
		}

		vk::Image GetCurrentSwapImage() const {
			return swapChainList[currentSwap]->image;
		}

		vk::PhysicalDevice GetPhysicalDevice() const {
			return gpu;
		}

		const vk::PhysicalDeviceProperties& GetDeviceProperties() const {
			return deviceProperties;
		}

		vk::Format GetSurfaceFormat() const {
			return surfaceFormat;
		}

		vk::Format GetDepthFormat() const;

	protected:		
		vk::ClearValue			defaultClearValues[2];
		vk::Viewport			defaultViewport;
		vk::Rect2D				defaultScissor;	
		vk::Rect2D				defaultScreenRect;			
		vk::RenderPass			defaultRenderPass;
		vk::RenderPassBeginInfo defaultBeginInfo;
		
		vk::DescriptorPool		defaultDescriptorPool;	//descriptor sets come from here!

		vk::CommandPool			commandPools[CommandBuffer::Type::MAX_BUFFERS];
		vk::Queue				queueTypes[CommandBuffer::Type::MAX_BUFFERS];

		vk::CommandBuffer		frameCmds;

		//Initialisation Info
		std::vector<const char*> deviceExtensions;
		std::vector<const char*> deviceLayers;
		std::vector<vk::QueueFamilyProperties> deviceQueueProps;

		std::vector<const char*>	instanceExtensions;
		std::vector<const char*>	instanceLayers;

		int majorVersion = 1;
		int minorVersion = 1;

		bool				autoTransitionFrameBuffer = true;
		bool				autoBeginDynamicRendering = true;

		UniqueVulkanTexture depthBuffer;

		VmaAllocatorCreateInfo	allocatorInfo;

	private: 
		void	InitCommandPools();
		bool	InitInstance();
		bool	InitPhysicalDevice();
		bool	InitGPUDevice();
		bool	InitSurface();
		void	InitMemoryAllocator();
		uint32_t	InitBufferChain(vk::CommandBuffer  cmdBuffer);

		bool	InitDeviceQueueIndices();
		bool	CreateDefaultFrameBuffers();

		virtual void SetupDevice(vk::PhysicalDeviceFeatures2& deviceFeatures) {}

		vk::Instance		instance;	//API Instance
		vk::PhysicalDevice	gpu;		//GPU in use

		vk::PhysicalDeviceProperties		deviceProperties;
		vk::PhysicalDeviceMemoryProperties	deviceMemoryProperties;

		vk::PipelineCache		pipelineCache;
		vk::Device				device;		//Device handle	

		vk::SurfaceKHR		surface;
		vk::Format			surfaceFormat;
		vk::ColorSpaceKHR	surfaceSpace;

		vk::Queue			presentQueue;

		uint32_t			gfxQueueIndex			= 0;
		uint32_t			computeQueueIndex		= 0;
		uint32_t			copyQueueIndex			= 0;
		uint32_t			gfxPresentIndex			= 0;
		uint32_t			numFrameBuffers			= 0;

		struct SwapChain {
			vk::Image			image;
			vk::ImageView		view;
			vk::CommandBuffer	frameCmds;
		};
		std::vector<SwapChain*> swapChainList;
		uint32_t				currentSwap = 0;
		vk::Framebuffer* frameBuffers = nullptr;

		vk::SwapchainKHR	swapChain;
		VmaAllocator		memoryAllocator;
	};
}