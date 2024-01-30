#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive		: enable

#include "Data.glslh"

layout (location = 0) in vec3 pos;

layout(push_constant) uniform PushConstantVert{
	int startingIndex;
};

void main() {
	int objectID 		= startingIndex + gl_InstanceIndex;
	mat4 modelMatrix  	= objectStates[objectID].modelMatrix;

   gl_Position 	= shadowMatrix * modelMatrix * vec4(pos.xyz, 1);
}
