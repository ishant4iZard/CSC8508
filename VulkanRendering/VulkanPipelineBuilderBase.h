/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanPipeline.h"
#include "VulkanUtils.h"


namespace NCL::Rendering::Vulkan {
	class VulkanRenderer;
	class VulkanShader;

	struct VulkanVertexSpecification;

	template <class T, class P>
	class PipelineBuilderBase	{
	public:

		T& WithLayout(vk::PipelineLayout pipeLayout) {
			layout = pipeLayout;
			pipelineCreate.setLayout(pipeLayout);
			return (T&)*this;
		}

		T& WithPushConstant(vk::ShaderStageFlags flags, uint32_t offset, uint32_t size) {
			allPushConstants.emplace_back(vk::PushConstantRange(flags, offset, size));
			return (T&)*this;
		}

		T& WithDescriptorSetLayout(uint32_t slot, vk::DescriptorSetLayout layout) {
			assert(slot < 32);
			if (slot >= allLayouts.size()) {
				vk::DescriptorSetLayout nullLayout = Vulkan::GetNullDescriptor(sourceDevice);
				while (allLayouts.size() <= slot) {
					allLayouts.push_back(nullLayout);
				}
			}
			allLayouts[slot] = layout;
			return (T&)*this;
		}

		T& WithDescriptorBuffers() {
			pipelineCreate.flags |= vk::PipelineCreateFlagBits::eDescriptorBufferEXT;
			return (T&)*this;
		}

		P& GetCreateInfo() {
			return pipelineCreate;
		}
	protected:
		PipelineBuilderBase(vk::Device device) {
			sourceDevice = device;
		}
		~PipelineBuilderBase() {}

	protected:
		P pipelineCreate;
		vk::PipelineLayout	layout;
		vk::Device			sourceDevice;

		std::vector< vk::DescriptorSetLayout> allLayouts;
		std::vector< vk::PushConstantRange> allPushConstants;
	};
}