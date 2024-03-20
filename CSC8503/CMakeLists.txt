if(NOT("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))

    set(PROJECT_NAME CSC8503)
    #set(PROJECT_NAME PS5Starter)

    ################################################################################
    # Source groups
    ################################################################################

    set(NetPlugins
        "OnlineSubsystemBase.h"
        "OnlineSubsystemBase.cpp"
        "steam.h"
        "steam.cpp"
    )
    source_group("NetPlugins" FILES ${NetPlugins})

    set(Header_Files
        #"NetworkedGame.h"
        "NetworkPlayer.h"
        "TutorialGame.h"
        "Projectile.h"
        "Hole.h"
        "PS5_Game.h"
        "BouncePad.h"
        "GravityWell.h"
        "Teleporter.h"
        "LevelFileLoaderBase.h"
        "WindowsLevelLoader.h"
        "LevelEnum.h"
        "LevelObjectEnum.h"
        "ApplicationState.h"
        #"MenuSystem.h"
        "Event.h"
        "EventType.h"
        "Singleton.h"
        "BaseTextureManager.h"
        "Light.h"
        "DirectionalLight.h"
        "Powerup.h"
        "DebugHUD.h"
        "PS5MenuSystem.h"
    )
    source_group("Header Files" FILES ${Header_Files})

    set(Source_Files
        "Main.cpp"
        #"NetworkedGame.cpp"
        "NetworkPlayer.cpp"
        "TutorialGame.cpp"
        "Projectile.cpp"
        "Hole.cpp"
        "PS5_Game.cpp"
        "BouncePad.cpp"
        "GravityWell.cpp"
        "Teleporter.cpp"
        "WindowsLevelLoader.cpp"
        "ApplicationState.cpp"
        #"MenuSystem.cpp"
        "Singleton.cpp"
        "BaseTextureManager.cpp"
        "Powerup.cpp"
        "DebugHUD.cpp"
        "PS5MenuSystem.cpp"
    )
    source_group("Source Files" FILES ${Source_Files})

    set(OGL_Rendering_Files
        "GameTechRenderer.cpp"
        "GameTechRenderer.h"
        "OGLTextureManager.h"
        "OGLTextureManager.cpp"
        "OglHdrFbo.h"
        "OglPostProcessingFbo.h"
    )
    source_group("OGL Rendering Files" FILES ${OGL_Rendering_Files})

    set(PS5_Rendering_Files
        "GameTechRendererInterface.h"
        "GameTechAGCRenderer.cpp"
        "GameTechAGCRenderer.h"
        "GameTechAGCModel.h"
        "GameTechAGCModel.cpp"
        "BumpAllocator.h"
        "FrameData.h"
    )
    source_group("PS5 Rendering Files" FILES ${PS5_Rendering_Files})

    set(AI_Files
        "AiStatemachineObject.cpp"
        "AiStatemachineObject.h"
    )
    source_group("AI_Files" FILES ${AI_Files})

      set(UIPlaystation
        "UIBase.h"
        "UIPlaystation.h"
        "UIPlaystation.cpp"
    )
    source_group("UIPlaystation" FILES ${UIPlaystation})

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

    file(GLOB_RECURSE ALL_SHADERS CONFIGURE_DEPENDS 
        ${COMPILE_ASSET_ROOT}Shaders/PSSL/*.pssl
    )
    source_group("Shader Files" FILES ${ALL_SHADERS})

    file(GLOB_RECURSE ALL_TEXTURES CONFIGURE_DEPENDS 
        ${COMPILE_ASSET_ROOT}Textures/*.jpg 
        ${COMPILE_ASSET_ROOT}Textures/*.png 
        ${COMPILE_ASSET_ROOT}Textures/*.dds 
    )
    source_group("Texture Files" FILES ${ALL_TEXTURES})

    # PS5 files
    set(ALL_FILES
        ${Header_Files}
        ${Source_Files}
        #${NetPlugins}
        ${AudioEngine}
        ${AI_Files}
        ${UIPlaystation}
        ${PS5_Rendering_Files}
        ${ALL_SHADERS}
        ${ALL_TEXTURES}
    )

    #file(GLOB_RECURSE Header_Files CONFIGURE_DEPENDS *.h)
    #source_group("Header Files" FILES ${Header_Files})
    #
    #file(GLOB_RECURSE Source_Files CONFIGURE_DEPENDS *.cpp)
    #source_group("Source Files" FILES ${Source_Files})
    #

    #set(ALL_FILES
    #    ${Header_Files}
    #    ${Source_Files}
    #	#${ALL_SHADERS}
    #	#${ALL_TEXTURES}
    #)

    ################################################################################
    # Target
    ################################################################################
    add_executable(${PROJECT_NAME} ${ALL_FILES})

    target_precompile_headers(${PROJECT_NAME} PRIVATE
        <memory>
        <unordered_set>
        <vector>
        <map>
        <set>
        <string>
        <fstream>
        <iostream>
        <iosfwd>

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

	    <Frustum.h>
        <Camera.h>
        <GameTimer.h>
        <TextureLoader.h>
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
	    VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)"
    )
    ################################################################################
    # Compile and link options
    ################################################################################
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
        )
    endif()

    ################################################################################
    # Dependencies
    ################################################################################
    include_directories("../NCLCoreClasses/")
    include_directories("../CSC8503CoreClasses/")
    include_directories("../PS5Core/")
    include_directories("../GLTFLoader/")
    include_directories("../Dependencies/FMOD/Playstation/include/")   
     
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "${CMAKE_SOURCE_DIR}/Dependencies/FMOD/Playstation/lib/libfmod_stub_weak.a")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "${CMAKE_SOURCE_DIR}/Dependencies/FMOD/Playstation/lib/libfmodL_stub_weak.a")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "${CMAKE_SOURCE_DIR}/Dependencies/FMOD/Playstation/lib/libfmodstudio_stub_weak.a")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "${CMAKE_SOURCE_DIR}/Dependencies/FMOD/Playstation/lib/libfmodstudioL_stub_weak.a")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC PS5Core)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC GLTFLoader)

    #target_link_libraries(${PROJECT_NAME} 
    #    PRIVATE "../Dependencies/FMOD/Playstation/lib/"
    #    PRIVATE libfmod_stub_weak
    #    PRIVATE libfmodL_stub_weak
    #)

    # Set the tool and GenerateHeader on PSSL files in ${PSSL_SHADERS}
    set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_TOOL_OVERRIDE WavePsslc)
    set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_SETTINGS "GenerateHeader=false")
    set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_SETTINGS "OutputFileName=${COMPILE_ASSET_ROOT}Shaders/PSSL/%(FileName).ags")
    set_property(SOURCE ${ALL_SHADERS} PROPERTY OUTPUT_NAME "$(Test)%(FileName).ags")


    # Set FileType to Document for all files using the WavePsslc tool
    set_property(TARGET CSC8503 PROPERTY VS_SOURCE_SETTINGS_WavePsslc "FileType=Document")	

    add_custom_target(
        Shaders
    )

    foreach (file ${ALL_TEXTURES})
        get_filename_component(file_name ${file} NAME)
	    get_filename_component(file_ext ${file} EXT)
	
	    message("Adding custom command to ${file}")
	    get_filename_component(file_dir ${file} ABSOLUTE)
	    #set(SPIRV_OUTPUT ${file_name}.spv)
	    set(TEX_INPUT ${file_dir})
	    set(TEX_OUTPUT ${file_dir}.gnf)
	
	    message("Reading from ${TEX_INPUT}")
	    message("Writing to ${TEX_OUTPUT}")
	
	    add_custom_command(
		    OUTPUT ${TEX_OUTPUT}
		
		    COMMENT "Generating GNF for texture ${file}"
		    COMMAND image2gnf.exe -g 1 -i "${file}" -o ${TEX_OUTPUT} -f Bc1UNorm
		    DEPENDS ${file}
		    VERBATIM
	    )
	    list(APPEND TEX_GNF_FILES ${TEX_OUTPUT})   
    endforeach()

    add_custom_target(
        Textures
	    DEPENDS ON ${TEX_GNF_FILES}
    )

    #set(PROJECT_DEPENDENCIES
    #    NCLCoreClasses
    #    PS5Core
    #    GLTFLoader
    #    #CSC8503
    #    CSC8503CoreClasses
    #)
    #
    #add_dependencies(${PROJECT_NAME}
    #    ${PROJECT_DEPENDENCIES}
    #    Shaders
	#    Textures
    #)

    target_link_libraries(${PROJECT_NAME} 
        PRIVATE ${PROJECT_DEPENDENCIES}
        PRIVATE SceAgcDriver_stub_weak
        PRIVATE SceAgc_stub_weak
        PRIVATE SceVideoOut_stub_weak
        PRIVATE SceAgc_debug_nosubmission
        PRIVATE SceAgcCore_debug_nosubmission
        PRIVATE SceAgcGpuAddress_debug_nosubmission
	    PRIVATE ScePad_stub_weak
	    PRIVATE SceUserService_stub_weak
    )

    include_directories(${COMPILE_ASSET_ROOT}) 
    

     # Function to copy files
    function(copy_files_from_source_to_destinations SOURCE_DIR DESTINATIONS FILE_PATTERN)
        message("Copying files from Source: " ${SOURCE_DIR})

        if ("${FILE_PATTERN}" MATCHES "\\*")
            file(GLOB_RECURSE FILES ${SOURCE_DIR}/**/${FILE_PATTERN})
        else()
            set(FILES ${SOURCE_DIR}/${FILE_PATTERN})
        endif()

        foreach(DEST_DIR ${DESTINATIONS})
            message("Copying files to Destination: " ${DEST_DIR})
            file(COPY ${FILES} DESTINATION ${DEST_DIR})
        endforeach()
    endfunction()

    # FMOD dlls
    set(FMOD_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Dependencies)
    set(FMOD_DESTINATIONS
        ${CMAKE_BINARY_DIR}/CSC8503
        ${CMAKE_SOURCE_DIR}/Prospero/Debug
        ${CMAKE_SOURCE_DIR}/Prospero/Release
    )
    copy_files_from_source_to_destinations(${FMOD_SOURCE_DIR} "${FMOD_DESTINATIONS}" "*.dll")

else()


    set(PROJECT_NAME CSC8503)

    ################################################################################
    # Source groups
    ################################################################################
    set(NetPlugins
        "OnlineSubsystemBase.h"
        "OnlineSubsystemBase.cpp"
        "steam.h"
        "steam.cpp"
    )
    source_group("NetPlugins" FILES ${NetPlugins})

    set(AudioEngine
        "AudioEngine.h"
        "AudioEngine.cpp"
    )
    source_group("AudioEngine" FILES ${AudioEngine})

    set(UI
        "UIBase.h"
        "UIWindows.h"
        "UIWindows.cpp"
    )
    source_group("UI" FILES ${UI})

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
        "Powerup.h"
        "LevelFileLoaderBase.h"
        "WindowsLevelLoader.h"
        "LevelEnum.h"
        "LevelObjectEnum.h"
        "ApplicationState.h"
        "MenuSystem.h"
        "Event.h"
        "EventType.h"
        "Singleton.h"
        "BaseTextureManager.h"
        "OGLTextureManager.h"
        "Light.h"
        "DirectionalLight.h"
        "DebugHUD.h"
        "ThreadPool.h"
        "Particles.h"
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
        "ApplicationState.cpp"
        "MenuSystem.cpp"
        "Singleton.cpp"
        "BaseTextureManager.cpp"
        "OGLTextureManager.cpp"
        "Powerup.cpp"
        "DebugHUD.cpp"
        "Particles.cpp"
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
        ${UI}
    )

    if(USE_VULKAN)
        set(ALL_FILES 
            ${Header_Files}
            ${Source_Files}
            ${NetPlugins}
            ${AudioEngine}
            ${SHADER_FILES}
            ${ImGUI_Files}
            ${UI}
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

    # Function to copy files
    function(copy_files_from_source_to_destinations SOURCE_DIR DESTINATIONS FILE_PATTERN)
        message("Copying files from Source: " ${SOURCE_DIR})

        if ("${FILE_PATTERN}" MATCHES "\\*")
            file(GLOB_RECURSE FILES ${SOURCE_DIR}/**/${FILE_PATTERN})
        else()
            set(FILES ${SOURCE_DIR}/${FILE_PATTERN})
        endif()

        foreach(DEST_DIR ${DESTINATIONS})
            message("Copying files to Destination: " ${DEST_DIR})
            file(COPY ${FILES} DESTINATION ${DEST_DIR})
        endforeach()
    endfunction()

    # FMOD dlls
    set(FMOD_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Dependencies)
    set(FMOD_DESTINATIONS
        ${CMAKE_BINARY_DIR}/CSC8503
        ${CMAKE_SOURCE_DIR}/x64/Debug
        ${CMAKE_SOURCE_DIR}/x64/Release
    )
    copy_files_from_source_to_destinations(${FMOD_SOURCE_DIR} "${FMOD_DESTINATIONS}" "*.dll")

    # Steam dlls
    set(STEAM_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Steamworksdk)
    set(STEAM_DESTINATIONS
        ${CMAKE_BINARY_DIR}/CSC8503
        ${CMAKE_SOURCE_DIR}/x64/Debug
        ${CMAKE_SOURCE_DIR}/x64/Release
    )
    copy_files_from_source_to_destinations(${STEAM_SOURCE_DIR} "${STEAM_DESTINATIONS}" "*.dll")

    # Steam app id
    set(APP_ID_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Steamworksdk)
    set(APP_ID_DESTINATIONS
        ${CMAKE_BINARY_DIR}/CSC8503
        ${CMAKE_SOURCE_DIR}/x64/Debug
        ${CMAKE_SOURCE_DIR}/x64/Release
    )
    copy_files_from_source_to_destinations(${APP_ID_SOURCE_DIR} "${APP_ID_DESTINATIONS}" "steam_appid.txt")

    # TODO : copy files to ORBIS
endif()