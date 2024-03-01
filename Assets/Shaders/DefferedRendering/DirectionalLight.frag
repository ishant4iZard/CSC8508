#version 400 core

uniform sampler2D diffuseMettalicTex;
uniform sampler2D normalRoughnessTex;
uniform sampler2D baseReflectivityAmbiantOccTex;
uniform sampler2D depthStencilTex;
uniform vec3 cameraPos;
uniform vec2 pixelSize;

// Directional Light
uniform vec3 globalLightDirection;
uniform vec4 lightColour;
uniform float globalIntensity;

in Vertex
{
    vec2 texCoord;
} IN;

out vec4 fragColour[2];
const float PI = 3.14159265359;

const mat4 inverseProjView = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);

//Returns the attenuation of a given light source position using linear formula, ideal for using in non-gamma corrected scene.
float CalculateLinearAttenuation(vec3 inLightPos, vec3 inWorldPos)
{
    return 1.0f - clamp(
        length(inLightPos - inWorldPos)/10.0f, 0.0f, 1.0f);
}

//Returns the attenuation of a given light source position using quaderitic formula, ideal for using in gamma corrected scene.
float CalculateInverseSquareAttenuation(vec3 inLightPos, vec3 inWorldPos)
{
    return 1.0f / (length(inLightPos - inWorldPos)) * (length(inLightPos - inWorldPos));
}

float CalculateSpotLightAttenuation(vec3 inLightPos, vec3 inLightDirection, vec3 inLightFragmentDirection, vec3 inWorldPos, float inConstant, float inLinear, float inQuadratic)
{
    float tempDistance = length(inLightPos - inWorldPos);
    return pow(max(dot(-inLightDirection, inLightFragmentDirection), 0), 8) /
     (inConstant +  (inLinear * tempDistance) + (inQuadratic * tempDistance * tempDistance));
}

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

vec3 BaseReflectivity(const vec3 inAlbedo, const float inMetallicFactor)
{
    return mix(vec3(0.04, 0.04, 0.04), inAlbedo, inMetallicFactor);
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
    vec3 wi = normalize(-globalLightDirection); //direction vector from sun to world
    vec3 wo = normalize(cameraPos - worldPos); //Direction vector from fragment to camera
    vec3 halfVector = normalize(wi + wo);
            
    vec3 radiance = lightColour.rgb * globalIntensity;

    // cook-torrance BRDF
    float NDF = DistributionGGX(normal, halfVector, roughness);       
    float G   = GeometrySmith(normal, wo, wi, roughness);
    vec3 F  = fresnelSchlick(max(dot(halfVector, wo), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, wo), 0.0) * max(dot(normal, wi), 0.0)  + 0.0001; //We add 0.0001 to the denominator to prevent a divide by zero in case any dot product ends up 0.0.
    vec3 specular     = (numerator / denominator);
    vec3 diffuse = (kD * albedo) / PI;
    //add to outgoing radiance lOut
    float NdotL = max(dot(normal, wi), 0.0);  

    fragColour[0].rgb = vec3((diffuse * radiance * NdotL));
    fragColour[0].a = ao;
    fragColour[1] = vec4((specular * radiance * NdotL), 1.0f);
}