#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive		: enable

#include "Data.glslh"

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec4 inColour;

layout (location = 0) out Vertex {
	vec4 colour;
	vec2 texCoord;
} OUT;

void main() {
	vec4 worldPos		= vec4(inPosition.xy, 0, 1);

	OUT.colour		= inColour;
	OUT.texCoord	= inTexCoord;

	gl_Position 	= orthoMatrix * worldPos;
}