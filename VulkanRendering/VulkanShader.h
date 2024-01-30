/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/Shader.h"

namespace NCL::Rendering::Vulkan {
	class VulkanShader : public Shader {
	public:
		friend class VulkanRenderer;
		friend class ShaderBuilder;

		void ReloadShader() override;

		void	FillShaderStageCreateInfo(vk::GraphicsPipelineCreateInfo& info) const;
		~VulkanShader();

	protected:
		void AddBinaryShaderModule(const std::string& fromFile, ShaderStages::Type stage, vk::Device device, const std::string& entryPoint = "main");

		void Init();

	protected:			
		VulkanShader();

		vk::UniqueShaderModule shaderModules[ShaderStages::MAX_SIZE];
		std::string entryPoints[ShaderStages::MAX_SIZE];

		uint32_t stageCount;
		vk::PipelineShaderStageCreateInfo* infos;
	};
}