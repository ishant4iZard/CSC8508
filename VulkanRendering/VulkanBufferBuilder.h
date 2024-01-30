/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanBuffers.h"

namespace NCL::Rendering::Vulkan {
	struct VulkanBuffer;

	class BufferBuilder	{
	public:
		BufferBuilder(vk::Device device, VmaAllocator allocator);

		BufferBuilder& WithBufferUsage(vk::BufferUsageFlags flags);
		BufferBuilder& WithMemoryProperties(vk::MemoryPropertyFlags flags);
		BufferBuilder& WithHostVisibility();
		BufferBuilder& WithDeviceAddress();
		BufferBuilder& WithPersistentMapping();

		BufferBuilder& WithUniqueAllocation();

		~BufferBuilder() {};

		VulkanBuffer Build(size_t byteSize, const std::string& name = "");

	protected:
		vk::Device sourceDevice;
		VmaAllocator sourceAllocator;
		VmaAllocationCreateInfo vmaInfo;
		vk::BufferCreateInfo	vkInfo;
	};
}
