/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanMesh.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "VulkanBufferBuilder.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

//These are both carefully arranged to match the MeshBuffer enum class!
vk::Format attributeFormats[] = {
	vk::Format::eR32G32B32Sfloat,	//Positions have this format
	vk::Format::eR32G32B32A32Sfloat,//Colours
	vk::Format::eR32G32Sfloat,		//TexCoords
	vk::Format::eR32G32B32Sfloat,	//Normals
	vk::Format::eR32G32B32A32Sfloat,//Tangents are 4D!
	vk::Format::eR32G32B32A32Sfloat,//Skel Weights
	vk::Format::eR32G32B32A32Sint,	//Skel indices
};
//Attribute sizes for each of the above
size_t attributeSizes[] = {
	sizeof(Vector3),
	sizeof(Vector4),
	sizeof(Vector2),
	sizeof(Vector3),
	sizeof(Vector4),
	sizeof(Vector4),
	sizeof(Vector4),
};

VulkanMesh::VulkanMesh()	{
}

//VulkanMesh::VulkanMesh(const std::string& filename) : Mesh(filename) {
//	debugName = filename;
//}

VulkanMesh::~VulkanMesh()	{
}

void VulkanMesh::UploadToGPU(RendererBase* r, vk::BufferUsageFlags extraUses) {
	assert(ValidateMeshData());

	//TODO: Do we care about supporting 16bit indices?
	if (GetIndexCount() > 0) {
		indexType = vk::IndexType::eUint32;
	}

	VulkanRenderer* renderer = (VulkanRenderer*)r;

	vk::Queue gfxQueue		= renderer->GetQueue(CommandBuffer::Graphics);
	vk::CommandPool pool	= renderer->GetCommandPool(CommandBuffer::Graphics);
	vk::Device device		= renderer->GetDevice();

	vk::UniqueCommandBuffer cmdBuffer = CmdBufferBegin(device, pool, "VulkanMesh upload");

	size_t allocationSize = CalculateGPUAllocationSize();

	VulkanBuffer stagingBuffer = BufferBuilder(renderer->GetDevice(), renderer->GetMemoryAllocator())
		.WithBufferUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.WithHostVisibility()
		.Build(allocationSize, "Staging Buffer");

	UploadToGPU(renderer, gfxQueue, *cmdBuffer, stagingBuffer, extraUses);

	CmdBufferEndSubmitWait(*cmdBuffer, device, gfxQueue);
	//The staging buffer is auto destroyed, but that's fine!
	//We made the GPU wait for the commands to complete, so 
	//the staging buffer has been read from at this point
}

void VulkanMesh::UploadToGPU(RendererBase* r)  {
	UploadToGPU(r, {});
}

void VulkanMesh::UploadToGPU(VulkanRenderer* renderer, VkQueue queue, vk::CommandBuffer cmdBuffer, VulkanBuffer& stagingBuffer, vk::BufferUsageFlags extraUses) {
	usedAttributes.clear();
	attributeBindings.clear();
	attributeDescriptions.clear();

	vk::Device sourceDevice = renderer->GetDevice();

	std::vector<const char*> attributeDataSources;//Pointer for each attribute in CPU memory

	size_t vSize = 0;

	auto atrributeFunc = [&](VertexAttribute::Type attribute, size_t count, const char* data) {
		if (count > 0) {
			usedAttributes.push_back(attribute);
			usedFormats.push_back(attributeFormats[attribute]);
			attributeDataSources.push_back(data);
			vSize += attributeSizes[attribute];
		}
	};

	atrributeFunc(VertexAttribute::Positions, GetPositionData().size(), (const char*)GetPositionData().data());
	atrributeFunc(VertexAttribute::Colours, GetColourData().size(), (const char*)GetColourData().data());
	atrributeFunc(VertexAttribute::TextureCoords, GetTextureCoordData().size(), (const char*)GetTextureCoordData().data());
	atrributeFunc(VertexAttribute::Normals, GetNormalData().size(), (const char*)GetNormalData().data());
	atrributeFunc(VertexAttribute::Tangents, GetTangentData().size(), (const char*)GetTangentData().data());
	atrributeFunc(VertexAttribute::JointWeights, GetSkinWeightData().size(), (const char*)GetSkinWeightData().data());
	atrributeFunc(VertexAttribute::JointIndices, GetSkinIndexData().size(), (const char*)GetSkinIndexData().data());

	for (uint32_t i = 0; i < usedAttributes.size(); ++i) {
		//Which vertex attribute slot should Vulkan buffer index i map to?
		int attributeType = usedAttributes[i];
		//Describes the vertex attribute state
		attributeBindings.emplace_back(i, (unsigned int)attributeSizes[attributeType], vk::VertexInputRate::eVertex);
		//Describes the vertex attribute data type and offset
		attributeDescriptions.emplace_back(attributeType, i, attributeFormats[attributeType], 0);

		attributeMask |= (1 << attributeType);
	}

	vertexInputState = vk::PipelineVertexInputStateCreateInfo({},
		(uint32_t)attributeBindings.size(), &attributeBindings[0],
		(uint32_t)attributeDescriptions.size(), &attributeDescriptions[0]
	);

	size_t vertexDataSize	= vSize * GetVertexCount();
	size_t indexDataSize	= sizeof(int) * GetIndexCount();
	size_t totalAllocationSize = vertexDataSize + indexDataSize;

	assert(stagingBuffer.size >= (totalAllocationSize));

	gpuBuffer = BufferBuilder(sourceDevice, renderer->GetMemoryAllocator())
		.WithBufferUsage(	vk::BufferUsageFlagBits::eVertexBuffer	| 
							vk::BufferUsageFlagBits::eIndexBuffer	| 
							vk::BufferUsageFlagBits::eTransferDst	| 
							vk::BufferUsageFlagBits::eStorageBuffer |
							extraUses)
		.Build(totalAllocationSize, debugName + " mesh Data");

	//need to now copy vertex data to device memory
	char* dataPtr = (char*)stagingBuffer.Map();
	size_t offset = 0;
	for (size_t i = 0; i < usedAttributes.size(); ++i) {
		//We're going to use the same buffer for every attribute
		usedBuffers.push_back(gpuBuffer.buffer);
		//But each attribute starts at a different offset
		usedOffsets.push_back(offset);
		//Copy the data from CPU to GPU-visible memory
		size_t copySize = GetVertexCount() * attributeSizes[usedAttributes[i]];
		memcpy(dataPtr + offset, attributeDataSources[i], copySize);
		offset += copySize;
	}
	
	if (GetIndexCount() > 0) {
		memcpy(dataPtr + offset, GetIndexData().data(), indexDataSize);
		indexType		= vk::IndexType::eUint32;	
		indexOffset		= offset;
	}
	stagingBuffer.Unmap();

	{//Now to transfer the mesh data from the staging buffer to the gpu-only buffer
		vk::BufferCopy copyRegion;
		copyRegion.size = vertexDataSize + indexDataSize;
		cmdBuffer.copyBuffer(stagingBuffer.buffer, gpuBuffer.buffer, copyRegion);
	}

}

void VulkanMesh::BindToCommandBuffer(vk::CommandBuffer  buffer) const {
	buffer.bindVertexBuffers(0, (unsigned int)usedBuffers.size(), &usedBuffers[0], &usedOffsets[0]);

	if (GetIndexCount() > 0) {
		buffer.bindIndexBuffer(gpuBuffer.buffer, indexOffset, indexType);
	}
}

vk::PrimitiveTopology VulkanMesh::GetVulkanTopology() const {
	assert((uint32_t)primType < GeometryPrimitive::MAX_PRIM);

	const vk::PrimitiveTopology table[] = {
		vk::PrimitiveTopology::ePointList,
		vk::PrimitiveTopology::eLineList,
		vk::PrimitiveTopology::eTriangleList,
		vk::PrimitiveTopology::eTriangleFan,
		vk::PrimitiveTopology::eTriangleStrip,
		vk::PrimitiveTopology::ePatchList
	};
	return table[(uint32_t)primType];
}

uint32_t VulkanMesh::GetAttributeMask() const {
	return attributeMask;
}

size_t VulkanMesh::CalculateGPUAllocationSize() const {
	size_t vSize = 0;
	auto atrributeSizeFunc = [&](VertexAttribute::Type attribute, size_t count) {
		if (count > 0) {
			vSize += (int)attributeSizes[attribute];
		}
	};

	atrributeSizeFunc(VertexAttribute::Positions, GetPositionData().size());
	atrributeSizeFunc(VertexAttribute::Colours, GetColourData().size());
	atrributeSizeFunc(VertexAttribute::TextureCoords, GetTextureCoordData().size());
	atrributeSizeFunc(VertexAttribute::Normals, GetNormalData().size());
	atrributeSizeFunc(VertexAttribute::Tangents, GetTangentData().size());
	atrributeSizeFunc(VertexAttribute::JointWeights, GetSkinWeightData().size());
	atrributeSizeFunc(VertexAttribute::JointIndices, GetSkinIndexData().size());

	size_t vertexDataSize = vSize * GetVertexCount();
	size_t indexDataSize = 0;

	if (GetIndexCount() > 0) {
		int elementSize = (indexType == vk::IndexType::eUint32) ? 4 : 2;
		indexDataSize = elementSize * GetIndexCount();
	}
	return vertexDataSize + indexDataSize;
}

bool VulkanMesh::GetIndexInformation(vk::Buffer& outBuffer, uint32_t& outOffset, uint32_t& outRange, vk::IndexType& outType) {
	if (indexType == vk::IndexType::eNoneKHR) {
		return false;
	}

	int elementSize = (indexType == vk::IndexType::eUint32) ? 4: 2;
	
	outBuffer	= gpuBuffer.buffer;
	outOffset	= indexOffset;
	outRange	= elementSize * GetIndexCount();
	outType		= indexType;

	return true;
}

bool VulkanMesh::GetAttributeInformation(VertexAttribute::Type v, vk::Buffer& outBuffer, uint32_t& outOffset, uint32_t& outRange, vk::Format& outFormat) const {
	for (uint32_t i = 0; i < usedAttributes.size(); ++i) {
		if (usedAttributes[i] != v) {
			continue;
		}

		outBuffer	= usedBuffers[i];
		outOffset	= usedOffsets[i];
		outRange	= attributeSizes[v] * GetVertexCount();
		outFormat	= usedFormats[i];

		return true;
	}

	return false;
}