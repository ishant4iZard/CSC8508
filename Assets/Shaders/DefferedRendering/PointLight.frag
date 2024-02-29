#version 400 core

uniform sampler2D diffuseMettalicTex;
uniform sampler2D normalRoughnessTex;
uniform sampler2D baseReflectivityAmbiantOccTex;
uniform sampler2D depthStencilTex;
uniform vec3 cameraPos;
uniform vec2 pixelSize;
uniform mat4 inverseProjView;

//Point Light
uniform vec4 lightColour;
const vec3 pointLightConstantLinearQuad = vec3(1.0f, 0.35f, 0.44);

out vec4 fragColour[2];

const float PI = 3.14159265359;

in Vertex
{
    vec3 lightWorldPos;
} IN;

float CalculatePointLightAttenuation(vec3 inLightPos, vec3 inWorldPos, float inConstant, float inLinear, float inQuadratic)
{
    float tempDistance = length(inLightPos - inWorldPos);
    return (
        (1.0f) /
          (inConstant + (inLinear * tempDistance) + (inQuadratic * tempDistance * tempDistance)));
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//Based on observations by Disney and adopted by Epic Games, the lighting looks more correct squaring the roughness in both the geometry and normal distribution function.
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

void main()
{
    const float gamma = 2.2;
    vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);

    float depth = texture(depthStencilTex, texCoord).r;
    vec3 ndcPos = vec3(texCoord, depth) * 2.0 - 1.0;
    vec4 invClipPos = inverseProjView * vec4(ndcPos, 1.0);
    vec3 worldPos = invClipPos.xyz / invClipPos.w;

    vec3 albedo = pow(texture(diffuseMettalicTex, texCoord).rgb, vec3(gamma)); //Albedo already broguht to linear space
    vec3 normal = texture(normalRoughnessTex, texCoord).rgb;
    float metallic = texture(diffuseMettalicTex, texCoord).a;
    float roughness = texture(normalRoughnessTex, texCoord).a;
    float ao = texture(baseReflectivityAmbiantOccTex, texCoord).a;
    vec3 F0 = texture(baseReflectivityAmbiantOccTex, texCoord).rgb; //surface reflection at zero incidence 

    //Calculate light radiance
    vec3 wi = normalize(IN.lightWorldPos - worldPos); //direction vector from fragmnet to lightSource
    vec3 wo = normalize(cameraPos - worldPos); //Direction vector from fragment to camera, in equation it's called Lo
    vec3 halfVector = normalize(wi + wo);

    //Calculate attenuation for POINT LIGHT
    float attenuation = CalculatePointLightAttenuation(IN.lightWorldPos, worldPos, pointLightConstantLinearQuad.x, pointLightConstantLinearQuad.y, pointLightConstantLinearQuad.z);
    
    vec3 radiance = lightColour.rgb * attenuation;

	// Specular reflection vector.
	vec3 Lr = 2.0 * (max(0.0, dot(normal, wo))) * normal - wo;

    // cook-torrance BRDF
    float NDF = DistributionGGX(normal, halfVector, roughness);       
    float G   = GeometrySmith(normal, wo, wi, roughness);
    vec3 F  = fresnelSchlick(dot(halfVector, wo), F0);

    vec3 kS = F;
    vec3 kD = mix(vec3(1.0) - F, vec3(0.0), metallic);

    // Lambert diffuse BRDF.
	vec3 diffuseBRDF = (kD * albedo) / PI;
    // Cook-Torrance specular microfacet BRDF.
    
    vec3 numerator    = NDF * G * F;
    float denominator = max(0.0001, 4.0 * max(dot(normal, wo), 0.0) * max(dot(normal, wi), 0.0)); //We add 0.0001 to the denominator to prevent a divide by zero in case any dot product ends up 0.0.
    vec3 specularBRDF = numerator / denominator;

    //add to outgoing radiance lOut
    float NdotL = max(dot(normal, wi), 0.0);
    vec3 ambient = vec3(0.33) * albedo * ao;
    fragColour[0] = vec4((diffuseBRDF * radiance * NdotL), 1.0f);
    fragColour[1] = vec4((specularBRDF * radiance * NdotL), 1.0f);
}