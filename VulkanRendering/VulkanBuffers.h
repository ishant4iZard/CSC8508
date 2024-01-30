/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "vma/vk_mem_alloc.h"

namespace NCL::Rendering::Vulkan {
	//A buffer, backed by memory we have allocated elsewhere
	struct VulkanBuffer {
		vk::Buffer	buffer;
		size_t		size;

		vk::DeviceAddress	deviceAddress;

		VmaAllocation		allocationHandle;
		VmaAllocationInfo	allocationInfo;
		VmaAllocator		allocator;

		VulkanBuffer() {
			buffer			= nullptr;
			size			= 0;
			deviceAddress	= 0;
		}

		VulkanBuffer(VulkanBuffer&& obj) {
			buffer = obj.buffer;
			deviceAddress = obj.deviceAddress;
			allocationHandle = obj.allocationHandle;
			allocationInfo = obj.allocationInfo;
			allocator = obj.allocator;
			size = obj.size;

			obj.buffer = VK_NULL_HANDLE;
		}

		VulkanBuffer& operator=(VulkanBuffer&& obj) {
			if (this != &obj) {
				buffer = obj.buffer;
				deviceAddress = obj.deviceAddress;
				allocationHandle = obj.allocationHandle;
				allocationInfo = obj.allocationInfo;
				allocator = obj.allocator;
				size = obj.size;

				obj.buffer = VK_NULL_HANDLE;
			}
			return *this;
		}

		~VulkanBuffer() {
			if (buffer) {
				vmaDestroyBuffer(allocator, buffer, allocationHandle);
			}
		}

		//A convenience func to help get around vma holding various
		//mapped pointers etc, so us calling mapBuffer can cause
		//validation errors
		void	CopyData(void* data, size_t size);

		void*	Data();

		void*	Map();
		template<typename T>
		T* Map() {
			void* data = Map();
			return static_cast<T*>(data);
		}

		void	Unmap();

		//Convenience function so we can use this struct in place of a vkBuffer when necessary
		operator vk::Buffer() const {
			return buffer;
		}
	};
};