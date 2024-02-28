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
    vec3 lightPos = vec3(instanceMatrix[3][0], instanceMatrix[3][1], instanceMatrix[3][2]);

    // Extract scale from the model matrix
    vec3 lightRadius;
    lightRadius.x = instanceMatrix[0][0]; // Length of the first row (x-axis scale)
    lightRadius.y = instanceMatrix[1][1]; // Length of the second row (y-axis scale)
    lightRadius.z = instanceMatrix[2][2]; // Length of the third row (z-axis scale)

    OUT.lightWorldPos = (position * vec3(lightRadius)) + lightPos;
    gl_Position = (projMatrix * viewMatrix) * vec4(OUT.lightWorldPos, 1.0f);
}