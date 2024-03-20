// Shader tutorial inspired from https://learnopengl.com/In-Practice/2D-Game/Particles
// Modified by Qixu Feng

#version 330 core

uniform sampler2D sprite;

in Vertex{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	vec4 color;
} IN;

out vec4 fragColor;

void main(void) {
	fragColor = (texture(sprite, IN.texCoord) * IN.color);
}