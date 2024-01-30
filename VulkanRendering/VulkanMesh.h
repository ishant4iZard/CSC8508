/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/Mesh.h"
#include "VulkanBuffers.h"

namespace NCL::Rendering::Vulkan {
	class VulkanMesh : public Mesh {
	public:
		friend class VulkanRenderer;
		VulkanMesh();
		//VulkanMesh(const std::string& filename);
		~VulkanMesh();

		const vk::PipelineVertexInputStateCreateInfo& GetVertexInputState() const {
			return vertexInputState;
		}

		void BindToCommandBuffer(vk::CommandBuffer  buffer) const;

		void UploadToGPU(RendererBase* renderer) override;

		void UploadToGPU(RendererBase* renderer, vk::BufferUsageFlags extraUses);
		void UploadToGPU(VulkanRenderer* renderer, VkQueue queue, vk::CommandBuffer buffer, VulkanBuffer& stagingBuffer, vk::BufferUsageFlags extraUses = {});

		uint32_t	GetAttributeMask() const;
		size_t		CalculateGPUAllocationSize() const;
		vk::PrimitiveTopology GetVulkanTopology() const;

		size_t GetVertexStride() const {
			return vertexStride;
		}

		operator const vk::PipelineVertexInputStateCreateInfo&() const {
			return vertexInputState;
		}

		bool GetIndexInformation(vk::Buffer& outBuffer, uint32_t& outOffset, uint32_t& outRange, vk::IndexType& outType);
		bool GetAttributeInformation(VertexAttribute::Type v, vk::Buffer& outBuffer, uint32_t& outOffset, uint32_t& outRange, vk::Format& outFormat) const;

	protected:
		vk::PipelineVertexInputStateCreateInfo				vertexInputState;
		std::vector<vk::VertexInputAttributeDescription>	attributeDescriptions;
		std::vector<vk::VertexInputBindingDescription>		attributeBindings;		
	
		VulkanBuffer gpuBuffer;
		size_t vertexOffset = 0;
		size_t indexOffset	= 0;

		size_t vertexStride = 0;

		uint32_t attributeMask = 0;

		vk::IndexType indexType = vk::IndexType::eNoneKHR;

		std::vector<vk::Buffer>					usedBuffers;
		std::vector<vk::DeviceSize>				usedOffsets;
		std::vector<vk::Format>					usedFormats;
		std::vector< VertexAttribute::Type >	usedAttributes;
	};
}