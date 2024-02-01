#version 330 core

uniform sampler2D diffuseTex;
in Vertex
{
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main()
{             
    vec3 hdrColor = texture(diffuseTex, IN.texCoord).rgb;
        
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
  
    fragColour = vec4(mapped, 1.0);
}