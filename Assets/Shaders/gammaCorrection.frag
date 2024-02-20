#version 400 core

uniform sampler2D diffuseTex;
in Vertex
{
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main()
{
    const float gamma = 2.2f;
    fragColour = pow(texture(diffuseTex, IN.texCoord), vec4(1.0f/ gamma));
}