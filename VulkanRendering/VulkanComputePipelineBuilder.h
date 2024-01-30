/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanPipeline.h"
#include "VulkanPipelineBuilderBase.h"
#include "SmartTypes.h"

namespace NCL::Rendering::Vulkan {
	class VulkanCompute;

	class ComputePipelineBuilder : public PipelineBuilderBase<ComputePipelineBuilder, vk::ComputePipelineCreateInfo>	{
	public:
		ComputePipelineBuilder(vk::Device device);
		~ComputePipelineBuilder() {}

		ComputePipelineBuilder& WithShader(UniqueVulkanCompute& shader);

		VulkanPipeline	Build(const std::string& debugName = "", vk::PipelineCache cache = {});

	protected:
	};
};