#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive		: enable

#include "Data.glslh"

layout (location = 0) in vec3 inPosition;
layout(location = 2) in vec2 texCoord;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out Vertex {
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	flat int texID;
} OUT;

layout(push_constant) uniform PushConstantVert{
	int startingIndex;
};

const mat4 biasMatrix = mat4( 
	0.5, 0.0, 0.0, 0.0, 
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
	int objectID = startingIndex + gl_InstanceIndex;

	mat4 modelMatrix  = objectStates[objectID].modelMatrix;
	mat4 mvp 		  = (projMatrix * viewMatrix * modelMatrix);
	mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));

	vec4 worldPos = ( modelMatrix * vec4 ( inPosition ,1));

	OUT.shadowProj 	=  biasMatrix * shadowMatrix * worldPos;
	OUT.worldPos 	= worldPos.xyz;
	OUT.normal 		= normalize ( normalMatrix * normalize ( inNormal ));

	//OUT.normal 		= inNormal;
	
	OUT.texCoord	= texCoord;
	OUT.colour		= objectStates[objectID].colour;
	OUT.texID		= objectStates[objectID].indices[0];

	gl_Position		= mvp * vec4(inPosition, 1.0);
}