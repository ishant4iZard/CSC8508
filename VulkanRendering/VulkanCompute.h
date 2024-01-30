/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::Rendering::Vulkan {
	class VulkanCompute	{
	public:
		VulkanCompute(vk::Device sourceDevice, const std::string& filename);
		~VulkanCompute() {}

		Maths::Vector3i GetThreadCount() const { return localThreadSize; }

		void	FillShaderStageCreateInfo(vk::ComputePipelineCreateInfo& info) const;

	protected:
		Maths::Vector3i localThreadSize;
		vk::PipelineShaderStageCreateInfo info;
		vk::UniqueShaderModule	computeModule;
	};
}