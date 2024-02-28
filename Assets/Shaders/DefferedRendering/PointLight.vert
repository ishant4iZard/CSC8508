#version 400 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

layout(location = 0) in vec3 position;
layout(location = 7) in mat4 instanceMatrix;

out Vertex
{
    vec3 lightWorldPos;
} OUT;

//LearnOPenGL Point Light

void main(void)
{
    vec3 lightPos = vec3(instanceMatrix[3][0], instanceMatrix[3][1], instanceMatrix[3][2]); //Light world position
    OUT.lightWorldPos = lightPos;
    
    vec3 lightRadius;
    lightRadius.x = instanceMatrix[0][0]; // Scale x (x-axis scale)
    lightRadius.y = instanceMatrix[1][1]; // Scale y (y-axis scale)
    lightRadius.z = instanceMatrix[2][2]; // Scale z (z-axis scale)

    vec3 lightVolumeWorldPos = (position * vec3(lightRadius)) + lightPos;
    gl_Position = (projMatrix * viewMatrix) * vec4(lightVolumeWorldPos, 1.0f);
}