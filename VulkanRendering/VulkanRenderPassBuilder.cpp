/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanRenderPassBuilder.h"
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

RenderPassBuilder::RenderPassBuilder(vk::Device device) {
	subPass.setPDepthStencilAttachment(nullptr);
	sourceDevice = device;
}

RenderPassBuilder& RenderPassBuilder::WithColourAttachment(VulkanTexture* texture, bool clear, vk::ImageLayout startLayout, vk::ImageLayout useLayout,  vk::ImageLayout endLayout) {
	allDescriptions.emplace_back(
		vk::AttachmentDescription()
		.setInitialLayout(startLayout)
		.setFinalLayout(endLayout)
		.setFormat(texture->GetFormat())
		.setLoadOp(clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
	);
	allReferences.emplace_back(vk::AttachmentReference((uint32_t)allReferences.size(), useLayout));

	return *this;
}

RenderPassBuilder& RenderPassBuilder::WithDepthAttachment(VulkanTexture* texture, bool clear, vk::ImageLayout startLayout, vk::ImageLayout useLayout, vk::ImageLayout endLayout) {
	allDescriptions.emplace_back(
		vk::AttachmentDescription()
		.setInitialLayout(startLayout)
		.setFinalLayout(endLayout)
		.setFormat(texture->GetFormat())
		.setLoadOp(clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad)
	);
	depthReference			= vk::AttachmentReference((uint32_t)allReferences.size(), useLayout);
	subPass.setPDepthStencilAttachment(&depthReference);
	return *this;
}

RenderPassBuilder& RenderPassBuilder::WithDepthStencilAttachment(VulkanTexture* texture, bool clear, vk::ImageLayout startLayout, vk::ImageLayout useLayout, vk::ImageLayout endLayout) {
	return WithDepthAttachment(texture, clear, startLayout, useLayout, endLayout); //we just get different default parameters!
}

vk::UniqueRenderPass RenderPassBuilder::Build(const std::string& debugName) {
	subPass.setColorAttachmentCount((uint32_t)allReferences.size())
		.setPColorAttachments(allReferences.data())
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

	vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount((uint32_t)allDescriptions.size())
		.setPAttachments(allDescriptions.data())
		.setSubpassCount(1)
		.setPSubpasses(&subPass);

	vk::UniqueRenderPass pass = sourceDevice.createRenderPassUnique(renderPassInfo);

	if (!debugName.empty()) {
		SetDebugName(sourceDevice, vk::ObjectType::eRenderPass, GetVulkanHandle(*pass), debugName);
	}

	return std::move(pass);
}