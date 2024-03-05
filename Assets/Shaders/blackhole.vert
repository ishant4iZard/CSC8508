#version 330 core

layout(location = 0) in vec3 Postion;
layout(location = 1) in vec2 texCoord;

out Vertex{
	vec2 texCoord;
} OUT;

void main() {
	gl_Position = vec4(Postion, 1.0);
	OUT.texCoord = texCoord;
}