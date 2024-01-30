/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanBuffers.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

void VulkanBuffer::CopyData(void* data, size_t size) {
	if (allocationInfo.pMappedData) {
		//we're already mapped, can just copy
		memcpy(allocationInfo.pMappedData, data, size);
	}
	else {
		//We should be able to safely map this?
		void* mappedData = nullptr;
		vmaMapMemory(allocator, allocationHandle, &mappedData);
		memcpy(mappedData, data, size);
		vmaUnmapMemory(allocator, allocationHandle);
	}
}

void* VulkanBuffer::Map() {
	if (allocationInfo.pMappedData) {
		return allocationInfo.pMappedData;
	}
	void* mappedData = nullptr;
	vmaMapMemory(allocator, allocationHandle, &mappedData);
	return mappedData;
}


void	VulkanBuffer::Unmap() {
	vmaUnmapMemory(allocator, allocationHandle);
}

void* VulkanBuffer::Data() {
	return allocationInfo.pMappedData;
}