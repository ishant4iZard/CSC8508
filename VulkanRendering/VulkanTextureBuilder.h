/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "SmartTypes.h"
#include "VulkanBuffers.h"

namespace NCL::Rendering::Vulkan {
	class TextureBuilder	{
	public:
		TextureBuilder(vk::Device device, VmaAllocator allocator);
		~TextureBuilder() {}

		TextureBuilder& WithFormat(vk::Format format);
		TextureBuilder& WithLayout(vk::ImageLayout layout);
		TextureBuilder& WithAspects(vk::ImageAspectFlags aspects);
		TextureBuilder& WithUsages(vk::ImageUsageFlags usages);
		TextureBuilder& WithPipeFlags(vk::PipelineStageFlags flags);

		TextureBuilder& WithCommandBuffer(vk::CommandBuffer buffer);
		TextureBuilder& WithQueue(vk::Queue queue);
		TextureBuilder& WithPool(vk::CommandPool pool);

		TextureBuilder& WithMips(bool state);
		TextureBuilder& WithDimension(uint32_t width, uint32_t height, uint32_t depth = 1);


		//Builds an empty texture
		UniqueVulkanTexture Build(const std::string& debugName = "");

		//Builds a texture loaded from file
		UniqueVulkanTexture BuildFromFile(const std::string& filename);

		//Builds an empty cubemap
		UniqueVulkanTexture BuildCubemap(const std::string& debugName = "");

		//Builds a cubemap from file set
		UniqueVulkanTexture BuildCubemapFromFile(
			const std::string& negativeXFile, const std::string& positiveXFile,
			const std::string& negativeYFile, const std::string& positiveYFile,
			const std::string& negativeZFile, const std::string& positiveZFile,	
			const std::string& debugName = "");


		//If processing textures via a cmd list provided, the builder doesn't
		bool	IsProcessing() const;
		bool	IsProcessing(const std::string& debugName) const;
		void	WaitForProcessing();

	protected:
		struct TextureJob {
			std::string		jobName;
			vk::Image		image;
			vk::Fence		workFence;
			VulkanBuffer	stagingBuffer;

			std::vector<char*>dataSrcs;

			TextureJob() {

			}

			TextureJob(TextureJob& other) {
				jobName = other.jobName;
				image = other.image;
				workFence = other.workFence;
				stagingBuffer = std::move(other.stagingBuffer);
				dataSrcs = std::move(other.dataSrcs);
			}

			TextureJob(TextureJob&& other) {
				jobName = other.jobName;
				image = other.image;
				workFence = other.workFence;
				stagingBuffer = std::move(other.stagingBuffer);
				dataSrcs = std::move(other.dataSrcs);
			}
		};

		void BeginTexture(const std::string& debugName, vk::UniqueCommandBuffer& uniqueBuffer, vk::CommandBuffer& usingBuffer);
		void EndTexture(const std::string& debugName, vk::UniqueCommandBuffer& uniqueBuffer, vk::CommandBuffer& usingBuffer, TextureJob& job, UniqueVulkanTexture& t);

		UniqueVulkanTexture	GenerateTexture(vk::CommandBuffer cmdBuffer, Maths::Vector3i dimensions, bool isCube, const std::string& debugName);

		void UploadTextureData(vk::CommandBuffer buffer, TextureJob& job, Maths::Vector3i dimensions, uint32_t channelCount, vk::ImageAspectFlags aspectFlags);

		NCL::Maths::Vector3i	requestedSize;
		uint32_t				layerCount;
		bool					generateMips;

		vk::Format				format;
		vk::ImageLayout			layout;
		vk::ImageAspectFlags	aspects;
		vk::ImageUsageFlags		usages;
		vk::PipelineStageFlags	pipeFlags;

		vk::Device			sourceDevice;
		VmaAllocator		sourceAllocator;

		vk::Queue			queue;
		vk::CommandPool		pool;
		vk::CommandBuffer	cmdBuffer;

		std::vector<TextureJob> activeJobs;
	};
}