set(PROJECT_NAME OpenGLRendering)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "OGLTexture.h"
    "OGLShader.h"
    "OGLRenderer.h"
    "OGLMesh.h"
    "OGLComputeShader.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "OGLTexture.cpp"
    "OGLShader.cpp"
    "OGLRenderer.cpp"
    "OGLMesh.cpp"
    "OGLComputeShader.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(GLAD
    "glad/gl.h"
    "glad/gl.c"
)
source_group("GLAD" FILES ${GLAD})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${GLAD}
)

set_source_files_properties(${ALL_FILES} PROPERTIES LANGUAGE CXX)
 
################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

#use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

set(ROOT_NAMESPACE OpenGLRendering)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <chrono>
    <memory>
    <unordered_set>
    <vector>
    <string>
    <fstream>
    <iostream>
    <string>
    <iosfwd>
    <set>
	<assert.h>

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

target_include_directories (${PROJECT_NAME} 
    PUBLIC ${CMAKE_SOURCE_DIR}/NCLCoreClasses
    PUBLIC ${CMAKE_SOURCE_DIR}/OpenGLRendering
)	
################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "NCLCoreClasses"
    "opengl32.lib"
)

target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")