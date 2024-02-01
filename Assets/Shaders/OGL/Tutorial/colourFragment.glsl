#version 330 core

in Vertex
{
    vec4 colour;
} IN;

out vec4 fragColour;

void main()
{
    fragColour = IN.colour;
}