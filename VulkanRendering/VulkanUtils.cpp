/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanUtils.h"
#include "VulkanTexture.h"
#include "VulkanBuffers.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

std::map<vk::Device, vk::DescriptorSetLayout > nullDescriptors;

vk::DynamicLoader NCL::Rendering::Vulkan::dynamicLoader;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

void Vulkan::SetDebugName(vk::Device device, vk::ObjectType t, uint64_t handle, const std::string& debugName) {
	device.setDebugUtilsObjectNameEXT(
		vk::DebugUtilsObjectNameInfoEXT()
		.setObjectType(t)
		.setObjectHandle(handle)
		.setPObjectName(debugName.c_str())
	);
};

void Vulkan::BeginDebugArea(vk::CommandBuffer b, const std::string& name) {
	vk::DebugUtilsLabelEXT labelInfo;
	labelInfo.pLabelName = name.c_str();

	b.beginDebugUtilsLabelEXT(labelInfo);
}

void Vulkan::EndDebugArea(vk::CommandBuffer b) {
	b.endDebugUtilsLabelEXT();
}

void Vulkan::SetNullDescriptor(vk::Device device, vk::DescriptorSetLayout layout) {
	nullDescriptors.insert({ device, layout });
}

vk::DescriptorSetLayout Vulkan::GetNullDescriptor(vk::Device device) {
	return nullDescriptors[device];
}

vk::AccessFlags Vulkan::DefaultAccessFlags(vk::ImageLayout forLayout) {
	if (forLayout == vk::ImageLayout::eTransferDstOptimal) {
		return vk::AccessFlagBits::eTransferWrite;
	}
	else if (forLayout == vk::ImageLayout::eTransferSrcOptimal) {
		return vk::AccessFlagBits::eTransferRead;
	}
	else if (forLayout == vk::ImageLayout::eColorAttachmentOptimal) {
		return vk::AccessFlagBits::eColorAttachmentWrite;
	}
	else if (forLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}
	else if (forLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead; //added last bit?!?
	}
	return vk::AccessFlagBits::eNone;
}

vk::AccessFlags2 Vulkan::DefaultAccessFlags2(vk::ImageLayout forLayout) {
	if (forLayout == vk::ImageLayout::eTransferDstOptimal) {
		return vk::AccessFlagBits2::eTransferWrite;
	}
	else if (forLayout == vk::ImageLayout::eTransferSrcOptimal) {
		return vk::AccessFlagBits2::eTransferRead;
	}
	else if (forLayout == vk::ImageLayout::eColorAttachmentOptimal) {
		return vk::AccessFlagBits2::eColorAttachmentWrite;
	}
	else if (forLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		return vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
	}
	else if (forLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		return vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eInputAttachmentRead; //added last bit?!?
	}
	return vk::AccessFlagBits2::eNone;
}

void	Vulkan::ImageTransitionBarrier(vk::CommandBuffer  cmdBuffer, vk::Image image, 
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout, 
	vk::ImageAspectFlags aspect, 
	vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage, 
	int mipLevel, int mipCount, int layer, int layerCount) {
	vk::ImageMemoryBarrier memoryBarrier = vk::ImageMemoryBarrier()
		.setSubresourceRange(vk::ImageSubresourceRange(aspect, mipLevel, mipCount, layer, layerCount))
		.setImage(image)
		.setOldLayout(oldLayout)
		.setNewLayout(newLayout)
		.setDstAccessMask(DefaultAccessFlags(newLayout));

	cmdBuffer.pipelineBarrier(srcStage, dstStage, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &memoryBarrier);

	//vk::ImageMemoryBarrier2 memoryBarrier2;
	//
	//memoryBarrier2.setImage(image)
	//	.setSubresourceRange(vk::ImageSubresourceRange(aspect, mipLevel, mipCount, layer, layerCount))
	//	.setOldLayout(oldLayout)
	//	.setNewLayout(newLayout)
	//	.setSrcStageMask(srcStage)
	//	.setDstStageMask()
	//	.setDstAccessMask((vk::AccessFlagBits2)DefaultAccessFlags(newLayout))
	//	

	//vk::DependencyInfo info;
	//info.imageMemoryBarrierCount = 1;
	//info.pImageMemoryBarriers = &memoryBarrier2;

	//cmdBuffer.pipelineBarrier2(info);
}

void Vulkan::TransitionUndefinedToColour(vk::CommandBuffer  buffer, vk::Image t) {
	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput);
}

void Vulkan::TransitionColourToPresent(vk::CommandBuffer  buffer, vk::Image t) {
	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eBottomOfPipe);
}

void Vulkan::TransitionColourToSampler(vk::CommandBuffer  buffer, vk::Image t) {
	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader);
}

void Vulkan::TransitionSamplerToColour(vk::CommandBuffer  buffer, vk::Image t) {
	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eColorAttachmentOutput);
}

void Vulkan::TransitionDepthToSampler(vk::CommandBuffer  buffer, vk::Image t, bool doStencil) {
	vk::ImageAspectFlags flags = doStencil ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eDepth;

	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilReadOnlyOptimal, flags,
		vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::PipelineStageFlagBits::eFragmentShader);
}

void Vulkan::TransitionSamplerToDepth(vk::CommandBuffer  buffer, vk::Image t, bool doStencil) {
	vk::ImageAspectFlags flags = doStencil ? vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil : vk::ImageAspectFlagBits::eDepth;

	ImageTransitionBarrier(buffer, t,
		vk::ImageLayout::eDepthStencilReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal, flags,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eEarlyFragmentTests);
}

bool Vulkan::MessageAssert(bool condition, const char* msg) {
	if (!condition) {
		std::cerr << msg << "\n";
	}
	return condition;
}

void	Vulkan::CmdBufferResetBegin(vk::CommandBuffer  buffer) {
	buffer.reset();
	buffer.begin(vk::CommandBufferBeginInfo());
}

void	Vulkan::CmdBufferResetBegin(const vk::UniqueCommandBuffer&  buffer) {
	buffer->reset();
	buffer->begin(vk::CommandBufferBeginInfo());
}

vk::UniqueCommandBuffer	Vulkan::CmdBufferCreate(vk::Device device, vk::CommandPool fromPool, const std::string& debugName) {
	vk::CommandBufferAllocateInfo bufferInfo = vk::CommandBufferAllocateInfo(fromPool, vk::CommandBufferLevel::ePrimary, 1);

	auto buffers = device.allocateCommandBuffersUnique(bufferInfo); //Func returns a vector!

	if (!debugName.empty()) {
		Vulkan::SetDebugName(device, vk::ObjectType::eCommandBuffer, Vulkan::GetVulkanHandle(*buffers[0]), debugName);
	}
	return std::move(buffers[0]);
}

vk::UniqueCommandBuffer	Vulkan::CmdBufferBegin(vk::Device device, vk::CommandPool fromPool, const std::string& debugName) {
	vk::UniqueCommandBuffer buffer = CmdBufferCreate(device, fromPool, debugName);
	vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
	buffer->begin(beginInfo);
	return std::move(buffer);
}

void	Vulkan::CmdBufferEndSubmit(vk::CommandBuffer  buffer, vk::Queue queue, vk::Fence fence, vk::Semaphore waitSemaphore, vk::Semaphore signalSempahore) {
	if (!buffer) {
		std::cout << __FUNCTION__ << " Submitting invalid buffer?\n";
		return;
	}
	buffer.end();

	vk::SubmitInfo submitInfo = vk::SubmitInfo();
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&buffer);

	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eTopOfPipe;

	if (waitSemaphore) {
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
	}
	if (signalSempahore) {
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSempahore;
	}

	queue.submit(submitInfo, fence);
}

void		Vulkan::CmdBufferEndSubmitWait(vk::CommandBuffer  buffer, vk::Device device, vk::Queue queue) {
	vk::Fence fence = device.createFence({});

	CmdBufferEndSubmit(buffer, queue, fence);

	if (device.waitForFences(1, &fence, true, UINT64_MAX) != vk::Result::eSuccess) {
		std::cout << __FUNCTION__ << " Device queue submission taking too long?\n";
	};

	device.destroyFence(fence);
}

void	Vulkan::WriteImageDescriptor(vk::Device device, vk::DescriptorSet set, int bindingNum, int subIndex, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout) {
	auto imageInfo = vk::DescriptorImageInfo()
		.setSampler(sampler)
		.setImageView(view)
		.setImageLayout(layout);

	auto descriptorWrite = vk::WriteDescriptorSet()
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDstSet(set)
		.setDstBinding(bindingNum)
		.setDstArrayElement(subIndex)
		.setDescriptorCount(1)
		.setPImageInfo(&imageInfo);

	device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void	Vulkan::WriteStorageImageDescriptor(vk::Device device, vk::DescriptorSet set, int bindingNum, int subIndex, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout) {
	auto imageInfo = vk::DescriptorImageInfo()
		.setSampler(sampler)
		.setImageView(view)
		.setImageLayout(layout);

	auto descriptorWrite = vk::WriteDescriptorSet()
		.setDescriptorType(vk::DescriptorType::eStorageImage)
		.setDstSet(set)
		.setDstBinding(bindingNum)
		.setDstArrayElement(subIndex)
		.setDescriptorCount(1)
		.setPImageInfo(&imageInfo);

	device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void	Vulkan::WriteBufferDescriptor(vk::Device device, vk::DescriptorSet set, int bindingSlot, vk::DescriptorType bufferType, vk::Buffer buff, size_t offset, size_t range) {
	auto descriptorInfo = vk::DescriptorBufferInfo()
		.setBuffer(buff)
		.setOffset(offset)
		.setRange(range > 0 ? range : VK_WHOLE_SIZE);

	auto descriptorWrites = vk::WriteDescriptorSet()
		.setDescriptorType(bufferType)
		.setDstSet(set)
		.setDstBinding(bindingSlot)
		.setDescriptorCount(1)
		.setPBufferInfo(&descriptorInfo);

	device.updateDescriptorSets(1, &descriptorWrites, 0, nullptr);
}

void	Vulkan::WriteTLASDescriptor(vk::Device device, vk::DescriptorSet set, int bindingSlot, vk::AccelerationStructureKHR tlas) {
	auto descriptorInfo = vk::WriteDescriptorSetAccelerationStructureKHR()
		.setAccelerationStructureCount(1)
		.setAccelerationStructures(tlas);

	auto descriptorWrites = vk::WriteDescriptorSet()
		.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR)
		.setDstSet(set)
		.setDstBinding(bindingSlot)
		.setDescriptorCount(1)
		.setPNext(&descriptorInfo);

	device.updateDescriptorSets(1, &descriptorWrites, 0, nullptr);
}

vk::UniqueDescriptorSet Vulkan::BuildUniqueDescriptorSet(vk::Device device, vk::DescriptorSetLayout  layout, vk::DescriptorPool pool, uint32_t variableDescriptorCount) {
	vk::DescriptorSetAllocateInfo allocateInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(pool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&layout);

	vk::DescriptorSetVariableDescriptorCountAllocateInfoEXT variableDescriptorInfo;

	if (variableDescriptorCount > 0) {
		variableDescriptorInfo.setDescriptorSetCount(1);
		variableDescriptorInfo.pDescriptorCounts = &variableDescriptorCount;
		allocateInfo.setPNext((const void*)&variableDescriptorInfo);
	}

	return std::move(device.allocateDescriptorSetsUnique(allocateInfo)[0]);
}

/*Descriptor Buffer Writing*/
void Vulkan::WriteBufferDescriptor(vk::Device device,
	const vk::PhysicalDeviceDescriptorBufferPropertiesEXT& props,
	void* descriptorBufferMemory,
	vk::DescriptorSetLayout layout,
	size_t layoutIndex,
	vk::DeviceAddress bufferAddress,
	size_t bufferSize
) {
	vk::DescriptorAddressInfoEXT address(bufferAddress, bufferSize);

	vk::DescriptorGetInfoEXT getInfo(vk::DescriptorType::eUniformBuffer, &address);

	vk::DeviceSize offset = device.getDescriptorSetLayoutBindingOffsetEXT(layout, layoutIndex);

	device.getDescriptorEXT(&getInfo, props.uniformBufferDescriptorSize, ((char*)descriptorBufferMemory) + offset);
}