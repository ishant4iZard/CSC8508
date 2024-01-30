/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/Texture.h"
#include "VulkanTextureBuilder.h"
#include "SmartTypes.h"

namespace NCL::Rendering::Vulkan {
	class VulkanRenderer;

	class VulkanTexture : public Texture	{
		friend class VulkanRenderer;
		friend class TextureBuilder;
	public:
		~VulkanTexture();

		vk::ImageView GetDefaultView() const {
			return *defaultView;
		}

		vk::Format GetFormat() const {
			return format;
		}

		vk::Image GetImage() const {
			return image;
		}

		//Allows us to pass a texture as vk type to various functions
		operator vk::Image() const {
			return image;
		}
		operator vk::ImageView() const {
			return *defaultView;
		}		
		operator vk::Format() const {
			return format;
		}

		void GenerateMipMaps(vk::CommandBuffer  buffer, 
								vk::ImageLayout endLayout = vk::ImageLayout::eShaderReadOnlyOptimal, 
								vk::PipelineStageFlags endFlags = vk::PipelineStageFlagBits::eFragmentShader);

		static size_t GetMaxMips(Vector2i dimensions) {
			return (size_t)std::floor(log2(float(std::min(dimensions.x, dimensions.y)))) + 1;
		}

	protected:
		VulkanTexture();

		vk::UniqueImageView		defaultView;
		vk::Image				image; //Don't use 'Unique', uses VMA
		vk::Format				format;
		vk::ImageAspectFlags	aspectType;

		VmaAllocation			allocationHandle;
		VmaAllocationInfo		allocationInfo;
		VmaAllocator			allocator;

		uint32_t mipCount;
		uint32_t layerCount;
	};
}