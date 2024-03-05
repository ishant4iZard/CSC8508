#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D normalTex;
uniform sampler2D albedoTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;
uniform sampler2D ambiantOccTex;


uniform vec3	lightPos;
uniform float	lightRadius;
uniform vec4	lightColour;

uniform sampler2DShadow shadowTex; //Shadow

uniform vec3 cameraPos;

in Vertex{
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
    vec3 tangent;
    vec3 binormal;
	vec3 worldPos;
} IN;

out vec4 fragColour;

const float PI = 3.14159265359;

vec3 GetNormalFromTexture()
{
    mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

    vec3 bumpNormal = texture(normalTex, IN.texCoord).xyz;
    return normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));    
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;

    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.

    float denom = NdotV * (1.0 - k) + k;
	
    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 BaseReflectivity(const vec3 inAlbedo, const float inMetallicFactor)
{
    return mix(vec3(0.04, 0.04, 0.04), inAlbedo, inMetallicFactor);
}

void main(void){
	const float gamma = 2.2;
    vec3 albedo = pow(texture(diffuseTex, IN.texCoord).rgb, vec3(gamma));
    vec3 normal = GetNormalFromTexture();
    float metallic = texture(metallicTex, IN.texCoord).r;
    float roughness = texture(roughnessTex, IN.texCoord).r;
    float ao = texture(ambiantOccTex, IN.texCoord).r;
    vec3 F0 = BaseReflectivity(albedo, metallic);

    vec3 Lo = vec3(0.0f);

    vec3 wo = normalize(cameraPos - IN.worldPos);

    //To do...
    //now there is only one light for test
    vec3 wi = normalize(lightPos - IN.worldPos);

    vec3 halfVector = normalize(wo + wi);

    float distance = length(lightPos - IN.worldPos);

    //Attenuation
    //The other attenuation methods would make the characters appear very dim.

    //float attenuation = 1.0f / (distance * distance);
    //float attenuation = 1.0f / distance;
    //float attenuation = 1.0f / (1.0f + (0.22f * distance));
    //float attenuation = 1.0f / (1.0f + (0.22f * distance) + 0.20f * distance * distance);
    float attenuation = 1.0f - clamp(distance/lightRadius , 0.0f, 1.0f);

    vec3 radiance = vec3(lightColour) * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(IN.normal, halfVector, roughness);
    float G = GeometrySmith(IN.normal, wo, wi, roughness);

	vec3 F = fresnelSchlick(max(dot(halfVector, wo), 0.0f), F0);

    vec3 nominator = NDF * G * F;

    float denominator = 4.0f * max(dot(IN.normal, wo), 0.0f) * max(dot(IN.normal, wi), 0.0f) +0.001f;

    vec3 specular = nominator / denominator;

	vec3 kS = F;
	vec3 kD = vec3(1.0f) - kS;
	kD *= 1.0 - metallic;
	
    // add to outgoing radiance Lo
    float NdotL = max(dot(IN.normal, wi), 0.0f);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    

    vec3 ambient = vec3(0.03f) * albedo * ao;
    vec3 color = max((ambient + Lo), vec3(0.0));

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    fragColour = vec4(color, 1.0);

    //start to mix with bump
    mat3 bumpTBN = mat3( normalize(IN.tangent),normalize(IN.binormal), normalize(IN.normal));
    vec4 diffuse = texture(diffuseTex , IN.texCoord);
    vec3 bumpNormal = texture(bumpTex, IN.texCoord).rgb;
	bumpNormal = normalize(bumpTBN * normalize(bumpNormal * 2.0 - 1.0));
    float lambert = max(dot(wi , IN.normal), 0.0f);

    float specFactor = clamp(dot(halfVector , IN.normal), 0.0f , 1.0f);
	specFactor = pow(specFactor , 60.0f);
    vec3 surface = (diffuse.rgb * lightColour.rgb);
    vec3 bumpColor = surface * lambert * attenuation;
    bumpColor+=(lightColour.rgb * specFactor) * attenuation * 0.33f;
    bumpColor+=surface * ambient;

    fragColour.rgb +=bumpColor;

    //fragColour = vec4(IN.tangent,1);
}