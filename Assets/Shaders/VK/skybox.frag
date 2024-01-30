#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive		: enable

#include "Data.glslh"

layout (location = 3) in vec3 viewDir;
layout (location = 0) out vec4 fragColor;

void main() {
   fragColor 	= texture(skyTex, viewDir);



}