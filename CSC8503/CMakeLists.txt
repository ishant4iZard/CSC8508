set(PROJECT_NAME CSC8503)

################################################################################
# Source groups
################################################################################
set(NetPlugins
    "steam.h"
    "steam.cpp"
)
source_group("NetPlugins" FILES ${NetPlugins})

set(AudioEngine
    "AudioEngine.h"
    "AudioEngine.cpp"
)
source_group("AudioEngine" FILES ${AudioEngine})

set(Header_Files
    "GameTechRenderer.h"
    "NetworkedGame.h"
    "NetworkPlayer.h"
    "TutorialGame.h"
    "Projectile.h"
    "Hole.h"
    "BouncePad.h"
    "GravityWell.h"
    "Teleporter.h"
    "UIBase.h"
    "LevelFileLoaderBase.h"
    "WindowsLevelLoader.h"
    "LevelEnum.h"
    "LevelObjectEnum.h"
    "UIWindows.h"
    "ApplicationState.h"
    "UIPlaystation.h"
    "MenuSystem.h"
    "Event.h"
    "EventType.h"
    "Singleton.h"
    "BaseTextureManager.h"
    "OGLTextureManager.h"
    "Light.h"
    "DirectionalLight.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "GameTechRenderer.cpp"
    "Main.cpp"
    "NetworkedGame.cpp"
    "NetworkPlayer.cpp"
    "TutorialGame.cpp"
    "Projectile.cpp"
    "Hole.cpp"
    "BouncePad.cpp"
    "GravityWell.cpp"
    "Teleporter.cpp"
    "WindowsLevelLoader.cpp"
    "UIWindows.cpp"
    "ApplicationState.cpp"
    "UIPlaystation.cpp"
    "MenuSystem.cpp"
    "Singleton.cpp"
    "BaseTextureManager.cpp"
    "OGLTextureManager.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(OGL_Rendering_Files
    "OglHdrFbo.h"
    "OglPostProcessingFbo.h"
)
source_group("OGL Rendering Files" FILES ${OGL_Rendering_Files})

set(AI_Files
    "AiStatemachineObject.cpp"
    "AiStatemachineObject.h"
    "AitreeObject.cpp"
    "AitreeObject.h"
)
source_group("AI_Files" FILES ${AI_Files})


if(USE_VULKAN)
list(APPEND Header_Files "GameTechVulkanRenderer.h")
list(APPEND Source_Files "GameTechVulkanRenderer.cpp")



file(GLOB SHADER_FILES ${ASSET_ROOT}/Shaders/VK/*.*)

endif()


set(ImGUI_Files
    "imconfig.h"
    "imgui.h"
    "imgui.cpp"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_impl_opengl3.cpp"
    "imgui_impl_opengl3.h"
    "imgui_impl_opengl3_loader.h"
    "imgui_impl_win32.h"
    "imgui_impl_win32.cpp"
    "imgui_internal.h"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
    "imstb_rectpack.h"
    "imstb_textedit.h"
    "imstb_truetype.h"
)
source_group("ImGUI_Files" FILES ${ImGUI_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${ImGUI_Files}
    ${NetPlugins}
    ${OGL_Rendering_Files}
    ${AudioEngine}
    ${AI_Files}
)

if(USE_VULKAN)
    set(ALL_FILES 
        ${Header_Files}
        ${Source_Files}
        ${NetPlugins}
        ${AudioEngine}
        ${SHADER_FILES}
        ${ImGUI_Files}
    )

foreach (file ${SHADER_FILES})
    get_filename_component(file_name ${file} NAME)
	get_filename_component(file_ext ${file} EXT)
	
	if(file_ext STREQUAL ".h" OR file_ext STREQUAL ".cpp")
		continue()
	endif()
		
	if( file_ext STREQUAL  ".vert" OR
		file_ext STREQUAL  ".frag" OR
		file_ext STREQUAL  ".comp" OR
		file_ext STREQUAL  ".geom" OR
		file_ext STREQUAL  ".tesc" OR
		file_ext STREQUAL  ".tese" OR
		file_ext STREQUAL  ".rgen" OR
		file_ext STREQUAL  ".rint" OR
		file_ext STREQUAL  ".rahit" OR
		file_ext STREQUAL  ".rchit" OR
		file_ext STREQUAL  ".rmiss" OR
		file_ext STREQUAL  ".rcall" OR
		file_ext STREQUAL  ".task" OR
		file_ext STREQUAL  ".mesh"
	)
		message("Adding custom command to ${file}")
		get_filename_component(file_dir ${file} ABSOLUTE)
        set(SPIRV_OUTPUT ${file_name}.spv)
		set(SPIRV_ABS_INPUT ${file_dir})
		set(SPIRV_ABS_OUTPUT ${file_dir}.spv)
		
		add_custom_command(
			OUTPUT ${SPIRV_ABS_OUTPUT}
			
			COMMENT "Compiling GLSL shader:"
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V  ${SPIRV_ABS_INPUT} -o ${SPIRV_ABS_OUTPUT}
            DEPENDS ${file}
			VERBATIM
		)
        list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUTPUT})   
	endif()
endforeach()

endif()

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME}  ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE CSC8503)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Compile definitions
################################################################################
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE;"
        "_UNICODE" 
        "WIN32_LEAN_AND_MEAN"
        "_WINSOCKAPI_"   
        "_WINSOCK2API_"
        "_WINSOCK_DEPRECATED_NO_WARNINGS"
    )
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <map>
    <stack>
    <list>   
	<set>   
	<string>
    <thread>
    <atomic>
    <functional>
    <iostream>
	<chrono>
	<sstream>
	
	"../NCLCoreClasses/Vector2i.h"
    "../NCLCoreClasses/Vector3i.h"
    "../NCLCoreClasses/Vector4i.h"
	
    "../NCLCoreClasses/Vector2.h"
    "../NCLCoreClasses/Vector3.h"
    "../NCLCoreClasses/Vector4.h"
    "../NCLCoreClasses/Quaternion.h"
    "../NCLCoreClasses/Plane.h"
    "../NCLCoreClasses/Matrix2.h"
    "../NCLCoreClasses/Matrix3.h"
    "../NCLCoreClasses/Matrix4.h"
	
    "../NCLCoreClasses/GameTimer.h"
)
 
if(USE_VULKAN)
target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vulkan/vulkan.hpp>
    <SmartTypes.h>
    <VulkanRenderer.h>
    <VulkanShader.h>
    <VulkanShaderBuilder.h>
    <VulkanTexture.h>
    <VulkanMesh.h>
    <VulkanPipelineBuilder.h>
    <VulkanDynamicRenderBuilder.h>

    <VulkanTextureBuilder.h>

    <VulkanDescriptorSetLayoutBuilder.h>
    <VulkanRenderPassBuilder.h>
    <VulkanCompute.h>
    <VulkanComputePipelineBuilder.h>
	<VulkanBufferBuilder.h>
)
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /permissive-;
        /std:c++latest;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
    )
endif()

################################################################################
# Dependencies
################################################################################
if(MSVC)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
endif()

include_directories("../OpenGLRendering/")
include_directories("../NCLCoreClasses/")
include_directories("../CSC8503CoreClasses/")
include_directories("../Steamworksdk/public/steam/")
include_directories("../Dependencies/FMOD/Windows/include/")

#link_directories("../Steamworksdk/redistributable_bin/win64/")

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC OpenGLRendering)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "../../Steamworksdk/redistributable_bin/win64/steam_api64")
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "../../Dependencies/FMOD/Windows/lib/fmod_vc")
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "../../Dependencies/FMOD/Windows/lib/fmodL_vc")


if(USE_VULKAN)
    include_directories("../VulkanRendering/")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC VulkanRendering)

    add_compile_definitions(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
    add_compile_definitions(VK_NO_PROTOTYPES)

    add_custom_target(
        SHADER_FILES
        DEPENDS ON ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${PROJECT_NAME} SHADER_FILES)
endif()


