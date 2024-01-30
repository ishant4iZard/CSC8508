/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanShader.h"
#include "Assets.h"

using std::ifstream;

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

//These have both been ordered to match the ShaderStages enum for easy lookup!
vk::ShaderStageFlagBits stageTypes[] = {
	vk::ShaderStageFlagBits::eVertex,
	vk::ShaderStageFlagBits::eFragment, 
	vk::ShaderStageFlagBits::eGeometry,
	vk::ShaderStageFlagBits::eTessellationControl,
	vk::ShaderStageFlagBits::eTessellationEvaluation,
	vk::ShaderStageFlagBits::eMeshNV
};

VulkanShader::VulkanShader()	{
	stageCount	= 0;
	infos		= nullptr;
}

VulkanShader::~VulkanShader()	{
	delete infos;
}

void VulkanShader::ReloadShader() {

}

void VulkanShader::AddBinaryShaderModule(const std::string& fromFile, ShaderStages::Type stage, vk::Device device, const std::string& entryPoint) {
	char* data;
	size_t dataSize = 0;
	Assets::ReadBinaryFile(Assets::SHADERDIR + "VK/" + fromFile, &data, dataSize);

	if (dataSize > 0) {
		shaderModules[stage] = device.createShaderModuleUnique(vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), dataSize, (uint32_t*)data));
	}
	else {
		std::cout << __FUNCTION__ << " Problem loading shader file " << fromFile << "!\n";
	}

	shaderFiles[stage] = fromFile;
	entryPoints[stage] = entryPoint;
}

void VulkanShader::Init() {
	stageCount = 0;
	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		if (shaderModules[i]) {
			stageCount++;
		}
	}
	infos = new vk::PipelineShaderStageCreateInfo[stageCount];

	uint32_t doneCount = 0;
	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		if (shaderModules[i]) {
			infos[doneCount].stage	= stageTypes[i];
			infos[doneCount].module = *shaderModules[i];
			infos[doneCount].pName	= entryPoints[i].c_str();

			doneCount++;
			if (doneCount >= stageCount) {
				break;
			}
		}
	}
}

void	VulkanShader::FillShaderStageCreateInfo(vk::GraphicsPipelineCreateInfo &info) const {
	info.setStageCount(stageCount);
	info.setPStages(infos);
}