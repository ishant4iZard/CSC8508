#version 400 core

uniform sampler2D diffuseMettalicTex;
uniform sampler2D diffuseLightTex;
uniform sampler2D specularLightTex;

in Vertex
{
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main()
{
    vec3 albedo = texture(diffuseMettalicTex, IN.texCoord).rgb;
    vec3 diffuseLight = texture(diffuseLightTex, IN.texCoord).rgb;
    vec3 specularLight = texture(specularLightTex, IN.texCoord).rgb;

    fragColour.rgb = vec3(0.0);
    fragColour.rgb += albedo * diffuseLight;
    fragColour.rgb += specularLight;
    fragColour.a = 1.0;
}