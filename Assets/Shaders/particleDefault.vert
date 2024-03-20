// Shader tutorial inspired from https://learnopengl.com/In-Practice/2D-Game/Particles
// Modified by Qixu Feng

#version 330 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);
uniform mat4 shadowMatrix 	= mat4(1.0f);
uniform vec3 offset;
uniform vec4 color;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;

uniform vec4 		objectColour = vec4(1,1,1,1);

uniform bool hasVertexColours = false;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	vec4 color;
} OUT;

void main(void)
{
	float scale = 10.0f;
	OUT.texCoord = texCoord;
	OUT.color = color;
	gl_Position = projMatrix * vec4((position * scale) + offset, 1.0);
}