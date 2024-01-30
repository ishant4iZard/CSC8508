#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive		: enable

#include "Data.glslh"

layout (location = 0) in Vertex {
	vec4 colour;
	vec2 texCoord;
} IN;

layout (location = 0) out vec4 fragColor;

layout(push_constant) uniform FontIndex{
	int fontIndex;
};

void main() {
	float alpha = texture(textures[fontIndex], IN.texCoord).r;
	
	if(alpha < 0.00001f) {
		discard;
	}
	fragColor 	= IN.colour;
}