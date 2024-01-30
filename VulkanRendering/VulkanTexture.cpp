/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "TextureLoader.h"
#include "VulkanUtils.h"
#include "VulkanBuffers.h"
#include "VulkanBufferBuilder.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

VulkanTexture::VulkanTexture() {
	mipCount	= 0;
	layerCount	= 0;
	format		= vk::Format::eUndefined;
}

VulkanTexture::~VulkanTexture() {
	if (image) {
		vmaDestroyImage(allocator, image, allocationHandle);
	}
}

void VulkanTexture::GenerateMipMaps(vk::CommandBuffer  buffer, vk::ImageLayout endLayout, vk::PipelineStageFlags endFlags) {
	for (int layer = 0; layer < layerCount; ++layer) {	
		ImageTransitionBarrier(buffer, image, 
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, 
			aspectType, 
			vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eTransfer, 
			0, 1, layer);
		
		for (uint32_t mip = 1; mip < mipCount; ++mip) {
			vk::ImageBlit blitData;
			blitData.srcSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(mip - 1)
				.setBaseArrayLayer(layer)
				.setLayerCount(1);
			blitData.srcOffsets[0] = vk::Offset3D(0, 0, 0);
			blitData.srcOffsets[1].x = std::max(dimensions.x >> (mip - 1), 1);
			blitData.srcOffsets[1].y = std::max(dimensions.y >> (mip - 1), 1);
			blitData.srcOffsets[1].z = 1;

			blitData.dstSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(mip)
				.setLayerCount(1)
				.setBaseArrayLayer(layer);
			blitData.dstOffsets[0] = vk::Offset3D(0, 0, 0);
			blitData.dstOffsets[1].x = std::max(dimensions.x >> mip, 1);
			blitData.dstOffsets[1].y = std::max(dimensions.y >> mip, 1);
			blitData.dstOffsets[1].z = 1;

			ImageTransitionBarrier(buffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, aspectType, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, mip, 1, layer);
			buffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blitData, vk::Filter::eLinear);
			ImageTransitionBarrier(buffer, image, vk::ImageLayout::eTransferSrcOptimal, endLayout, aspectType, vk::PipelineStageFlagBits::eTransfer, endFlags, mip - 1, 1, layer);

			if (mip < this->mipCount - 1) {
				ImageTransitionBarrier(buffer, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal, aspectType, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, mip, 1, layer);
			}
			else {
				ImageTransitionBarrier(buffer, image, vk::ImageLayout::eTransferDstOptimal, endLayout, aspectType, vk::PipelineStageFlagBits::eTransfer, endFlags, mip, 1, layer);
			}
		}
	}
}
