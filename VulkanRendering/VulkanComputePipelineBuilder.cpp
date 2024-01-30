/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanComputePipelineBuilder.h"
#include "VulkanCompute.h"
#include "VulkanUtils.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

ComputePipelineBuilder::ComputePipelineBuilder(vk::Device device) : PipelineBuilderBase(device){
}

ComputePipelineBuilder& ComputePipelineBuilder::WithShader(UniqueVulkanCompute& compute) {
	compute->FillShaderStageCreateInfo(pipelineCreate);
	return *this;
}

VulkanPipeline	ComputePipelineBuilder::Build(const std::string& debugName, vk::PipelineCache cache) {
	VulkanPipeline output;

	vk::PipelineLayoutCreateInfo pipeLayoutCreate = vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount((uint32_t)allLayouts.size())
		.setPSetLayouts(allLayouts.data())
		.setPPushConstantRanges(allPushConstants.data())
		.setPushConstantRangeCount((uint32_t)allPushConstants.size());

	output.layout = sourceDevice.createPipelineLayoutUnique(pipeLayoutCreate);

	pipelineCreate.setLayout(*output.layout);

	output.pipeline = sourceDevice.createComputePipelineUnique(cache, pipelineCreate).value;

	if (!debugName.empty()) {
		SetDebugName(sourceDevice, vk::ObjectType::ePipeline, GetVulkanHandle(*output.pipeline), debugName);
	}

	return output;
}