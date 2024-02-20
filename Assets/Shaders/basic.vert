#version 330 core
//Uniforms
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

//Buffer objects
layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;
out Vertex
{
    vec2 texCoord;
} OUT;

void main()
{
    mat4 mvp = projMatrix * viewMatrix * modelMatrix;
    gl_Position = mvp * vec4(position, 1.0);
    OUT.texCoord = texCoord;
}