set(PROJECT_NAME VulkanRendering)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "VulkanUtils.h"
    "VulkanBuffers.h"
    "VulkanCompute.h"
    "VulkanComputePipelineBuilder.h"
    "VulkanDescriptorSetLayoutBuilder.h"
    "VulkanDynamicRenderBuilder.h"
	"VulkanTextureBuilder.h"
    "VulkanMesh.h"
    "VulkanPipeline.h"
	"VulkanPipelineBuilderBase.h"
    "VulkanPipelineBuilder.h"
    "VulkanRenderer.h"
    "VulkanRenderPassBuilder.h"
    "VulkanShader.h"
    "VulkanShaderBuilder.h"
    "VulkanTexture.h"
	"VulkanBufferBuilder.h"
	"SmartTypes.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
	"VulkanUtils.cpp"
    "VulkanBuffers.cpp"
    "VulkanCompute.cpp"
    "VulkanComputePipelineBuilder.cpp"
    "VulkanDescriptorSetLayoutBuilder.cpp"
    "VulkanDynamicRenderBuilder.cpp"
	"VulkanTextureBuilder.cpp"
    "VulkanMesh.cpp"
    "VulkanPipelineBuilder.cpp"
    "VulkanRenderer.cpp"
    "VulkanRenderPassBuilder.cpp"
    "VulkanShader.cpp"
    "VulkanShaderBuilder.cpp"
	"VulkanBufferBuilder.cpp"
    "VulkanTexture.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vulkan/vulkan.hpp>
	<vma/vk_mem_alloc.h>
    <memory>
    <unordered_set>
    <map>
    <vector>
    <set>
    <string>
    <fstream>
    <iostream>
    <string>
    <iosfwd>
    
    <Matrix2.h>
    <Matrix3.h>
    <Matrix4.h>

    <Vector2.h>
    <Vector3.h>
    <Vector4.h>

    <Vector2i.h>
    <Vector3i.h>
    <Vector4i.h>

    <Quaternion.h>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

add_compile_definitions(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
add_compile_definitions(VK_NO_PROTOTYPES)


################################################################################
# Compile and link options
################################################################################
target_include_directories (${PROJECT_NAME} 
    PUBLIC ${VULKAN_INCLUDE_DIRS}
)	

target_include_directories (${PROJECT_NAME} 
    PUBLIC ${Vulkan_INCLUDE_DIR}
    PUBLIC ${CMAKE_SOURCE_DIR}/NCLCoreClasses
)	

################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})