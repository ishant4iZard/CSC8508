/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::Rendering::Vulkan {
	class DynamicRenderBuilder	{
	public:
		DynamicRenderBuilder();
		~DynamicRenderBuilder() {}

		DynamicRenderBuilder& WithColourAttachment(
			vk::ImageView	texture,
			vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal,
			bool clear = true,
			vk::ClearValue clearValue = vk::ClearColorValue(std::array<float, 4>{0, 0, 0, 1})
		);

		DynamicRenderBuilder& WithDepthAttachment(
			vk::ImageView	texture,
			vk::ImageLayout layout = vk::ImageLayout::eDepthAttachmentOptimal,
			bool clear = true,
			vk::ClearValue clearValue = vk::ClearDepthStencilValue(1.0f, 0),
			bool withStencil = false
		);

		DynamicRenderBuilder& WithSecondaryBuffers();

		DynamicRenderBuilder& WithRenderArea(vk::Rect2D area);

		DynamicRenderBuilder& WithLayerCount(int count);

		DynamicRenderBuilder& BeginRendering(vk::CommandBuffer  buffer);
	protected:
		vk::RenderingInfoKHR renderInfo;
		std::vector< vk::RenderingAttachmentInfoKHR > colourAttachments;
		vk::RenderingAttachmentInfoKHR depthAttachment;
		bool		usingStencil;
		uint32_t	layerCount;
		vk::Rect2D	renderArea;
	};
}