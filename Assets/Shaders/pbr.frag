#version 400 core

uniform sampler2D 	mainTex;
uniform sampler2D diffuseTex;
uniform sampler2D normalTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;
uniform sampler2D ambiantOccTex;

uniform vec3	lightPos;
uniform float	lightRadius;
uniform vec4	lightColour;

//uniform sampler2DShadow shadowTex; //Shadow

uniform vec3 cameraPos;

// Directional Light
uniform vec3 globalLightDirection;
uniform vec4 globalLightColor;
uniform float globalIntensity;

//Point Light
uniform vec3 lightColorList[4];
uniform vec3 lightPosList[4];
uniform vec3 pointLightConstantLinearQuadList[4];

//Spot Light
uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform float spotLightCutoffAngle;

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

out vec4 fragColour;

const float PI = 3.14159265359;

vec3 GetNormalFromTexture()
{
    mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

    vec3 bumpNormal = texture(normalTex, IN.texCoord).xyz;
    return normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));    
}

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

vec3 SpotLight(vec3 inPos, vec3 inDir, float inCutoffAngle, vec3 inColor, vec3 normal, float roughness, float metallic, vec3 albedo)
{
    vec3 lOut = vec3(0.0);

    //Calculate light radiance
    vec3 tempSpotDir = normalize(inDir); //direction vector from light source to world
    vec3 tempLightDir = normalize(inPos - IN.worldPos); //Direction vector from fragment to spot light

    float theta =  degrees(acos(dot(tempSpotDir, -tempLightDir)));
    if(theta > inCutoffAngle) return lOut; 

    vec3 F0 = vec3(0.04); //surface reflection at zero incidence  
    F0 = mix(F0, albedo, metallic);

    vec3 halfVector = (tempSpotDir + tempLightDir) / length(tempSpotDir + tempLightDir);
    
    vec3 radiance = inColor * CalculateSpotLightAttenuation(inPos, tempSpotDir, tempLightDir, IN.worldPos, 1.0f, 0.22f, 0.20f);

    // cook-torrance BRDF
    float NDF = DistributionGGX(normal, halfVector, roughness);       
    float G   = GeometrySmith(normal, tempLightDir, tempSpotDir, roughness);
    vec3 F  = fresnelSchlick(max(dot(halfVector, tempLightDir), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, tempLightDir), 0.0) * max(dot(normal, tempSpotDir), 0.0)  + 0.0001; //We add 0.0001 to the denominator to prevent a divide by zero in case any dot product ends up 0.0.
    vec3 specular     = numerator / denominator;

    //add to outgoing radiance lOut
    float NdotL = max(dot(normal, tempSpotDir), 0.0);  
    lOut += (kD * albedo / PI + specular) * radiance * NdotL;
    return lOut;    
}

vec3 DirectionalLightPbr(vec3 inDirection, vec3 inWorldPos, vec3 inColor, vec3 inNormal, float inRoughness, float inMetallic, vec3 inAlbedo, vec3 inCameraPos)
{
    //Calculate light radiance
    vec3 wi = normalize(-inDirection); //direction vector from sun to world
    vec3 wo = normalize(inCameraPos - inWorldPos); //Direction vector from fragment to camera
    vec3 halfVector = normalize(wi + wo);
            
    vec3 radiance = inColor * globalIntensity;

    // cook-torrance BRDF
    float NDF = DistributionGGX(inNormal, halfVector, inRoughness);       
    float G   = GeometrySmith(inNormal, wo, wi, inRoughness);
    vec3 F0 = vec3(0.04f);
    vec3 F  = fresnelSchlick(max(dot(halfVector, wo), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - inMetallic);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(inNormal, wo), 0.0) * max(dot(inNormal, wi), 0.0)  + 0.0001; //We add 0.0001 to the denominator to prevent a divide by zero in case any dot product ends up 0.0.
    vec3 specular     = (numerator / denominator);

    //add to outgoing radiance lOut
    float NdotL = max(dot(inNormal, wi), 0.0);  
    return ((kD * inAlbedo / PI + specular) * radiance * NdotL); 
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

vec3 BaseReflectivity(const vec3 inAlbedo, const float inMetallicFactor)
{
    return mix(vec3(0.04, 0.04, 0.04), inAlbedo, inMetallicFactor);
}

void main()
{
    const float gamma = 2.2;
    vec3 albedo = pow(texture(diffuseTex, IN.texCoord).rgb, vec3(gamma));

    vec3 normal = GetNormalFromTexture();
    float metallic = texture(metallicTex, IN.texCoord).r;
    float roughness = texture(roughnessTex, IN.texCoord).r;
    float ao = texture(ambiantOccTex, IN.texCoord).r;
    vec3 F0 = BaseReflectivity(albedo, metallic); //surface reflection at zero incidence  
      
    float shadow = 1.0; // New !
	
	// if( IN.shadowProj . w > 0.0) { // New !
	// 	shadow = textureProj ( shadowTex , IN . shadowProj ) * 0.5f;
	// }
    
     // reflectance equation
    vec3 lOut = vec3(0.0);

    // Calculate spot light
    //lOut += SpotLight(spotLightPosition, spotLightDirection, spotLightCutoffAngle, spotLightColor, normal, roughness, metallic, albedo);

    // Calculate point light
    for(int i = 0; i < 1 ; ++i)
    {
        //lOut += PointLightPbr(lightPosList[i], IN.worldPos, lightColorList[i], normal, roughness, metallic, albedo, pointLightConstantLinearQuadList[i].x, pointLightConstantLinearQuadList[i].y, pointLightConstantLinearQuadList[i].z, cameraPos, F0);
        lOut += PointLightPbr(vec3(0,60,0), IN.worldPos, vec3(255, 255, 51), normal, roughness, metallic, albedo,
          1.0f, 0.22f, 0.20f, cameraPos, F0);
    }

    lOut += DirectionalLightPbr(globalLightDirection, IN.worldPos, globalLightColor.xyz, normal, roughness, metallic, albedo, cameraPos);
    
    vec3 ambient = vec3(0.33) * albedo * ao;
    vec3 color = max((ambient + lOut), vec3(0.0));
    fragColour = vec4(color, 1.0);
}