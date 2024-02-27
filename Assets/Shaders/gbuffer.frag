#version 400 core

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;
uniform sampler2D ambiantOccTex;

in Vertex
{
    vec4 colour;
    vec2 texCoord;
	vec4 shadowProj;
    vec3 normal;
    vec3 tangent; //New
    vec3 binormal; //New
    vec3 worldPos;
} IN;

out vec4 fragColour[5];

vec3 GetNormalFromTexture()
{
    mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

    vec3 bumpNormal = texture(normalTex, IN.texCoord).xyz;
    return normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));    
}

vec3 BaseReflectivity(const vec3 inAlbedo, const float inMetallicFactor)
{
    return mix(vec3(0.04, 0.04, 0.04), inAlbedo, inMetallicFactor);
}

void main()
{
    fragColour[0].rgb = texture(diffuseTex, IN.texCoord).rgb; //albedo
    fragColour[0].a = texture(metallicTex, IN.texCoord).r; //mettalic

    fragColour[1].rgb = GetNormalFromTexture(); //normal
    fragColour[1].a = texture(roughnessTex, IN.texCoord).r; //Roughness
    
    fragColour[2].rgb = BaseReflectivity(fragColour[0].rgb, fragColour[0].a); //surface reflection at zero incidence
    fragColour[2].a = texture(ambiantOccTex, IN.texCoord).r; //Ambiant occulision

    fragColour[3].rgb = IN.worldPos; //Position
}