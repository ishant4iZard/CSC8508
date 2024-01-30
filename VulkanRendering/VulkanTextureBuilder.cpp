/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanTextureBuilder.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"
#include "VulkanBufferBuilder.h"
#include "TextureLoader.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

TextureBuilder::TextureBuilder(vk::Device inDevice, VmaAllocator inAllocator) {
    sourceDevice    = inDevice;
    sourceAllocator = inAllocator;

    generateMips    = true;

    format      = vk::Format::eR8G8B8A8Unorm;
    layout      = vk::ImageLayout::eShaderReadOnlyOptimal;
    usages      = vk::ImageUsageFlagBits::eSampled;
    aspects     = vk::ImageAspectFlagBits::eColor;
    pipeFlags   = vk::PipelineStageFlagBits::eFragmentShader;

    layerCount      = 1;
}

TextureBuilder& TextureBuilder::WithFormat(vk::Format inFormat) {
    format = inFormat;
    return *this;
}

TextureBuilder& TextureBuilder::WithLayout(vk::ImageLayout inLayout) {
    layout = inLayout;
    return *this;
}

TextureBuilder& TextureBuilder::WithAspects(vk::ImageAspectFlags inAspects) {
    aspects = inAspects;
    return *this;
}

TextureBuilder& TextureBuilder::WithUsages(vk::ImageUsageFlags inUsages) {
    usages = inUsages;
    return *this;
}

TextureBuilder& TextureBuilder::WithPipeFlags(vk::PipelineStageFlags inFlags) {
    pipeFlags = inFlags;
    return *this;
}

TextureBuilder& TextureBuilder::WithDimension(uint32_t inWidth, uint32_t inHeight, uint32_t inDepth) {
	requestedSize = { (int)inWidth, (int)inHeight, (int)inDepth };
    return *this;
}

TextureBuilder& TextureBuilder::WithMips(bool inMips) {
    generateMips = inMips;
    return *this;
}

TextureBuilder& TextureBuilder::WithCommandBuffer(vk::CommandBuffer inBuffer) {
    assert(MessageAssert(queue == 0 && pool == 0, "Builder is either passed a command buffer OR uses a queue and pool!"));
    cmdBuffer = inBuffer;
    return *this;
}

TextureBuilder& TextureBuilder::WithQueue(vk::Queue inQueue) {
    assert(MessageAssert(cmdBuffer == 0, "Builder is either passed a command buffer OR uses a queue and pool!"));
    queue = inQueue;
    return *this;
}

TextureBuilder& TextureBuilder::WithPool(vk::CommandPool inPool) {
    assert(MessageAssert(cmdBuffer == 0, "Builder is either passed a command buffer OR uses a queue and pool!"));
    pool = inPool;
    return *this;
}

UniqueVulkanTexture TextureBuilder::Build(const std::string& debugName) {
    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;
    BeginTexture(debugName, uniqueBuffer, usingBuffer);

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, requestedSize, false, debugName);

    ImageTransitionBarrier(usingBuffer, tex->GetImage(), vk::ImageLayout::eUndefined, layout, aspects, vk::PipelineStageFlagBits::eTopOfPipe, pipeFlags);

    TextureJob job;
    job.image = tex->GetImage();
    EndTexture(debugName, uniqueBuffer, usingBuffer, job, tex);

    return tex;
}

void TextureBuilder::BeginTexture(const std::string& debugName, vk::UniqueCommandBuffer& uniqueBuffer, vk::CommandBuffer& usingBuffer) {
    //We're appending to an external command buffer
    if (cmdBuffer) {
        usingBuffer = cmdBuffer;
    }
    //We're in charge of our own command buffers
    else if (queue && pool) {
        uniqueBuffer = Vulkan::CmdBufferBegin(sourceDevice, pool, debugName + " Creation");
        usingBuffer = *uniqueBuffer;
    }
    else {
        assert(MessageAssert(true, "Cannot build without either a command buffer OR a queue and pool"));
    }
}

void TextureBuilder::EndTexture(const std::string& debugName, vk::UniqueCommandBuffer& uniqueBuffer, vk::CommandBuffer& usingBuffer, TextureJob& job, UniqueVulkanTexture& t) {
    if (generateMips) {
        int mipCount = VulkanTexture::GetMaxMips(t->GetDimensions());
        if (mipCount > 1) {
            t->mipCount = mipCount;
            t->GenerateMipMaps(usingBuffer);
        }
    }

    //If we're in charge of our own buffers, we just stop and wait for completion now
    if (uniqueBuffer) {
        CmdBufferEndSubmitWait(usingBuffer, sourceDevice, queue);
        for (const auto& i : job.dataSrcs) {
            TextureLoader::DeleteTextureData(i);
        }
    }
    //Otherwise, this is going to be handled external to the builder, and placed as a 'job'
    else {
        job.workFence   = sourceDevice.createFence({});
        job.jobName     = debugName;
        activeJobs.emplace_back(job);
    }
}

UniqueVulkanTexture TextureBuilder::BuildFromFile(const std::string& filename) {
    char* texData = nullptr;
    Vector3i dimensions(0, 0, 1);
    int channels    = 0;
    int flags       = 0;
    TextureLoader::LoadTexture(filename, texData, dimensions.x, dimensions.y, channels, flags);

    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;

    BeginTexture(filename, uniqueBuffer, usingBuffer);

    vk::ImageUsageFlags	realUsages = usages;

    usages |= vk::ImageUsageFlagBits::eTransferDst;

    if (generateMips) {
        usages |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, dimensions, false, filename);

    TextureJob job;
    job.dataSrcs = {texData};
    job.image = tex->GetImage();

    UploadTextureData(usingBuffer, job, dimensions, channels, vk::ImageAspectFlagBits::eColor);

    EndTexture(filename, uniqueBuffer, usingBuffer, job, tex);

    usages = realUsages;

    return tex;
}

UniqueVulkanTexture TextureBuilder::BuildCubemapFromFile(
    const std::string& negativeXFile, const std::string& positiveXFile,
    const std::string& negativeYFile, const std::string& positiveYFile,
    const std::string& negativeZFile, const std::string& positiveZFile,
    const std::string& debugName) {

    TextureJob job;
    job.dataSrcs.resize(6);

    Vector3i dimensions[6]{ Vector3i(0, 0, 1) };
    int channels[6] = { 0 };
    int flags[6] = { 0 };

    const std::string* filenames[6] = {
        &negativeXFile,
        &positiveXFile,
        &negativeYFile,
        &positiveYFile,
        &negativeZFile,
        &positiveZFile
    };

    for (int i = 0; i < 6; ++i) {
        TextureLoader::LoadTexture(*filenames[i], job.dataSrcs[i], dimensions[i].x, dimensions[i].y, channels[i], flags[i]);
    }

    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;
    BeginTexture(debugName, uniqueBuffer, usingBuffer);

    vk::ImageUsageFlags	realUsages = usages;

    usages |= vk::ImageUsageFlagBits::eTransferDst;

    if (generateMips) {
        usages |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, dimensions[0], true, debugName);
    job.image = tex->GetImage();

    UploadTextureData(usingBuffer, job, dimensions[0], channels[0], vk::ImageAspectFlagBits::eColor);
    EndTexture(debugName, uniqueBuffer, usingBuffer, job, tex);

    usages = realUsages;

    return tex;
}

UniqueVulkanTexture TextureBuilder::BuildCubemap(const std::string& debugName) {
    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;
    BeginTexture(debugName, uniqueBuffer, usingBuffer);

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, requestedSize, true, debugName);

    TextureJob job;
    job.image = tex->GetImage();
    EndTexture(debugName, uniqueBuffer, usingBuffer, job, tex);

    return tex;
}

UniqueVulkanTexture	TextureBuilder::GenerateTexture(vk::CommandBuffer cmdBuffer, Vector3i dimensions, bool isCube, const std::string& debugName) {
    VulkanTexture* t = new VulkanTexture();

    uint32_t mipCount = VulkanTexture::GetMaxMips(dimensions);

    auto createInfo = vk::ImageCreateInfo()
        .setImageType(dimensions.z > 1 ? vk::ImageType::e3D : vk::ImageType::e2D)
        .setExtent(vk::Extent3D(dimensions.x, dimensions.y, dimensions.z))
        .setFormat(format)
        .setUsage(usages)
        .setMipLevels(generateMips ? mipCount : 1)
        .setArrayLayers(1);

	if (isCube) {
		createInfo.setArrayLayers(6)
                  .setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
		layerCount = 6;
	}

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaCreateImage(sourceAllocator, (VkImageCreateInfo*)&createInfo, &vmaallocInfo, (VkImage*)&t->image, &t->allocationHandle, &t->allocationInfo);

    vk::ImageViewType viewType = vk::ImageViewType::e2D;
    if (isCube) {
        viewType = vk::ImageViewType::eCube;
    }
    else if (dimensions.z > 1) {
        viewType = vk::ImageViewType::e3D;
    }

	vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
        .setViewType(viewType)
		.setFormat(format)
        .setSubresourceRange(vk::ImageSubresourceRange(aspects, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS))
		.setImage(t->image);

	t->defaultView  = sourceDevice.createImageViewUnique(viewInfo);
    t->allocator    = sourceAllocator;
    t->layerCount   = layerCount;
    t->aspectType   = aspects;
    t->format       = format;
    t->dimensions   = { dimensions.x, dimensions.y };

	SetDebugName(sourceDevice, vk::ObjectType::eImage, GetVulkanHandle(t->image), debugName);
	SetDebugName(sourceDevice, vk::ObjectType::eImageView, GetVulkanHandle(*t->defaultView), debugName);

	ImageTransitionBarrier(cmdBuffer, t->image, vk::ImageLayout::eUndefined, layout, aspects, vk::PipelineStageFlagBits::eTopOfPipe, pipeFlags);

    return UniqueVulkanTexture(t);
}

void TextureBuilder::UploadTextureData(vk::CommandBuffer buffer, TextureJob& job, Vector3i dimensions, uint32_t channelCount, vk::ImageAspectFlags aspectFlags) {
    int faceSize = dimensions.x * dimensions.y * channelCount * sizeof(char); //TODO: Will loaded textures always be 8bpp?
    int allocationSize = faceSize * (int)job.dataSrcs.size();

    job.stagingBuffer = BufferBuilder(sourceDevice, sourceAllocator)
        .WithBufferUsage(vk::BufferUsageFlagBits::eTransferSrc)
        .WithHostVisibility()
        .Build(allocationSize, "Staging Buffer");

    //our buffer now has memory! Copy some texture date to it...
    char* gpuPtr = (char*)job.stagingBuffer.Map();
    for (int i = 0; i < job.dataSrcs.size(); ++i) {
        memcpy(gpuPtr, job.dataSrcs[i], faceSize);
        gpuPtr += faceSize;
        
  
    }
    job.stagingBuffer.Unmap();
    //We'll also set up each layer of the image to accept new transfers
    ImageTransitionBarrier(buffer, job.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, aspectFlags, vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, 0, 1, 0);
    vk::BufferImageCopy copyInfo;
    copyInfo.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor).setMipLevel(0).setLayerCount((uint32_t)job.dataSrcs.size());
    copyInfo.imageExtent = vk::Extent3D(dimensions.x, dimensions.y, 1);

    buffer.copyBufferToImage(job.stagingBuffer.buffer, job.image, vk::ImageLayout::eTransferDstOptimal, copyInfo);

    ImageTransitionBarrier(buffer, job.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, aspectFlags, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, 0, 1);
}

bool	TextureBuilder::IsProcessing() const {
    return !activeJobs.empty();
}

bool	TextureBuilder::IsProcessing(const std::string& debugName) const {
    if (activeJobs.empty()) {
        return false;
    }
    return false;
}

void	TextureBuilder::WaitForProcessing() {
    for (const auto& i : activeJobs) {
        if (sourceDevice.waitForFences(1, &i.workFence, true, UINT64_MAX) != vk::Result::eSuccess) {
        };
        sourceDevice.destroyFence(i.workFence);

        for (int j = 0; j < 6; ++j) {
            TextureLoader::DeleteTextureData(i.dataSrcs[j]);
        }
    }
    activeJobs.clear();
}