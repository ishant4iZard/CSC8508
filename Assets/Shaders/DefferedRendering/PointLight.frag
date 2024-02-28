#version 400 core

uniform sampler2D diffuseMettalicTex;
uniform sampler2D normalRoughnessTex;
uniform sampler2D baseReflectivityAmbiantOccTex;
uniform sampler2D fragmentPosition;
uniform vec3 cameraPos;

//Point Light
uniform vec4 lightColour;
uniform vec3 pointLightConstantLinearQuad;

in Vertex
{
    vec3 lightWorldPos;
} IN;

out vec4 fragColour;

const float PI = 3.14159265359;

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

vec3 PointLightPbr(vec3 inPos, vec3 inWorldPos, vec3 inColor, vec3 inNormal, float inRoughness, float inMetallic, vec3 inAlbedo, float inConstant, float inLinear, float inQuadratic, vec3 inCameraPos, vec3 inF0)
{
    //Calculate light radiance
    vec3 wi = normalize(inPos - inWorldPos); //direction vector from fragmnet to lightSource
    vec3 wo = normalize(inCameraPos - inWorldPos); //Direction vector from fragment to camera, in equation it's called Lo
    vec3 halfVector = normalize(wi + wo);

    //Calculate attenuation for POINT LIGHT
    float attenuation = CalculatePointLightAttenuation(inPos, inWorldPos, inConstant, inLinear, inQuadratic);
        
    vec3 radiance = inColor * attenuation;

	// Specular reflection vector.
	vec3 Lr = 2.0 * (max(0.0, dot(inNormal, wo))) * inNormal - wo;

    // cook-torrance BRDF
    float NDF = DistributionGGX(inNormal, halfVector, inRoughness);       
    float G   = GeometrySmith(inNormal, wo, wi, inRoughness);
    vec3 F  = fresnelSchlick(dot(halfVector, wo), inF0);

    vec3 kS = F;
    vec3 kD = mix(vec3(1.0) - F, vec3(0.0), inMetallic);

    // Lambert diffuse BRDF.
	vec3 diffuseBRDF = (kD * inAlbedo) / PI;

    // Cook-Torrance specular microfacet BRDF.
    
    vec3 numerator    = NDF * G * F;
    float denominator = max(0.0001, 4.0 * max(dot(inNormal, wo), 0.0) * max(dot(inNormal, wi), 0.0)); //We add 0.0001 to the denominator to prevent a divide by zero in case any dot product ends up 0.0.
    vec3 specularBRDF = numerator / denominator;

    //add to outgoing radiance lOut
    float NdotL = max(dot(inNormal, wi), 0.0);  
    return (diffuseBRDF + specularBRDF) * radiance * NdotL;
}

void main()
{
    // const float gamma = 2.2;
    // vec3 albedo = pow(texture(diffuseMettalicTex, IN.texCoord).rgb, vec3(gamma)); //Albedo already broguht to linear space
    // vec3 normal = texture(normalRoughnessTex, IN.texCoord).rgb;
    // float metallic = texture(diffuseMettalicTex, IN.texCoord).a;
    // float roughness = texture(normalRoughnessTex, IN.texCoord).a;
    // float ao = texture(baseReflectivityAmbiantOccTex, IN.texCoord).a;
    // vec3 F0 = texture(baseReflectivityAmbiantOccTex, IN.texCoord).rgb; //surface reflection at zero incidence  
      
    //  // reflectance equation
    // vec3 lOut = vec3(0.0);

    // lOut += PointLightPbr(lightPosList[i], texture(fragmentPosition, IN.texCoord).rgb, lightColorList[i], normal, roughness, metallic, albedo, pointLightConstantLinearQuadList[i].x, pointLightConstantLinearQuadList[i].y, pointLightConstantLinearQuadList[i].z, cameraPos, F0);

    // vec3 ambient = vec3(0.33) * albedo * ao;
    // vec3 color = max((ambient + lOut), vec3(0.0));
    fragColour = vec4(lightColour.rgb, 1.0);
}