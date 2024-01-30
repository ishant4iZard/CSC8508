#version 450
#extension GL_ARB_separate_shader_objects  	: enable
#extension GL_ARB_shading_language_420pack 	: enable
#extension GL_GOOGLE_include_directive		: enable
#include "Data.glslh"

layout (location = 0) in vec3 inPosition;

layout (location = 3) out vec3 viewDir;

void main() {
	vec4 worldPos	= vec4(inPosition.xyz, 1);

	mat4 invProj = inverse(projMatrix);

	viewDir.xy = inPosition.xy * vec2(invProj[0][0], invProj[1][1]);
	viewDir.z = -1.0f;

	viewDir = transpose(mat3(viewMatrix)) * normalize(viewDir);

	gl_Position 	= worldPos;
}