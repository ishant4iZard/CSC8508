/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanDynamicRenderBuilder.h"
#include "VulkanUtils.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

DynamicRenderBuilder::DynamicRenderBuilder() {
	usingStencil	= false;
	layerCount		= 1;
}

DynamicRenderBuilder& DynamicRenderBuilder::WithColourAttachment(
	vk::ImageView	texture, vk::ImageLayout layout, bool clear, vk::ClearValue clearValue
)
{
	vk::RenderingAttachmentInfoKHR colourAttachment;
	colourAttachment.setImageView(texture)
		.setImageLayout(layout)
		.setLoadOp(clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setClearValue(clearValue);

	colourAttachments.push_back(colourAttachment);

	return *this;
}

DynamicRenderBuilder& DynamicRenderBuilder::WithDepthAttachment(
	vk::ImageView	texture, vk::ImageLayout layout, bool clear, vk::ClearValue clearValue, bool withStencil
)
{
	depthAttachment.setImageView(texture)
		.setImageLayout(layout)
		.setLoadOp(clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setClearValue(clearValue);

	usingStencil = withStencil;

	return *this;
}

DynamicRenderBuilder& DynamicRenderBuilder::WithRenderArea(vk::Rect2D area) {
	renderArea = area;
	return *this;
}

DynamicRenderBuilder& DynamicRenderBuilder::WithLayerCount(int count) {
	layerCount = count;
	return *this;
}

DynamicRenderBuilder& DynamicRenderBuilder::WithSecondaryBuffers() {
	renderInfo.flags |= vk::RenderingFlagBits::eContentsSecondaryCommandBuffers;
	return *this;
}

DynamicRenderBuilder& DynamicRenderBuilder::BeginRendering(vk::CommandBuffer  buffer) {
	renderInfo.setLayerCount(layerCount)
		.setRenderArea(renderArea)
		.setColorAttachments(colourAttachments)
		.setPDepthAttachment(&depthAttachment);

	if (usingStencil) {
		renderInfo.setPStencilAttachment(&depthAttachment);
	}

	buffer.beginRendering(renderInfo);
	return *this;
}