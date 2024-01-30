#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive	   : enable

#include "Data.glslh"

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColour;

layout (location = 0) out Vertex {
	vec4 colour;
} OUT;

void main() {
	vec4 worldPos	= vec4(inPosition.xyz, 1);
	mat4 mvp 		  = (projMatrix * viewMatrix);

	OUT.colour		= inColour;

	gl_Position 	= mvp * worldPos;
}