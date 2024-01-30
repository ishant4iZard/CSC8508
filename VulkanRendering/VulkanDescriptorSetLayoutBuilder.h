/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::Rendering::Vulkan {
	class VulkanRenderer;

	class DescriptorSetLayoutBuilder {
	public:
		DescriptorSetLayoutBuilder(vk::Device device) {
			sourceDevice = device;
			//debugName = name; 
			//usingBindless = false; 
			//usingDescriptorBuffer = false;
		};
		~DescriptorSetLayoutBuilder() {};

		DescriptorSetLayoutBuilder& WithSamplers(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		
		DescriptorSetLayoutBuilder& WithSampledImages(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		DescriptorSetLayoutBuilder& WithStorageImages(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithUniformBuffers(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		DescriptorSetLayoutBuilder& WithStorageBuffers(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithDynamicUniformBuffers(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);
		DescriptorSetLayoutBuilder& WithDynamicStorageBuffers(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithAccelStructures(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithDescriptors(vk::DescriptorType type, unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		DescriptorSetLayoutBuilder& WithCreationFlags(vk::DescriptorSetLayoutCreateFlags flags);

		vk::UniqueDescriptorSetLayout Build(const std::string& debugName = "");

	protected:
		vk::DescriptorSetLayoutBinding& AddDescriptors(unsigned int count, vk::ShaderStageFlags inShaders = vk::ShaderStageFlagBits::eAll, vk::DescriptorBindingFlags = (vk::DescriptorBindingFlags)0);

		vk::Device sourceDevice;

		//bool usingBindless;
		//bool usingDescriptorBuffer;
		std::vector< vk::DescriptorSetLayoutBinding>	addedBindings;
		std::vector< vk::DescriptorBindingFlags>		addedFlags;

		vk::DescriptorSetLayoutCreateInfo createInfo;
	};
}