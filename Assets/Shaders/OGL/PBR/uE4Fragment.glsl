#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;
uniform sampler2D metallicTex;
uniform sampler2D roughnessTex;
uniform sampler2D ambiantOccTex;

uniform vec3 cameraPos;
uniform vec3 lightColorList[4];
uniform vec3 lightPosList[4];

in Vertex
{
    vec4 colour;
    vec2 texCoord;
    vec3 normal;
    vec3 tangent;
    vec3 binormal;
    vec3 worldPos;
} IN;

out vec4 fragColour;

const float PI = 3.14159265359;

vec3 GetNormalFromTexture()
{
    mat3 TBN = mat3(normalize(IN.tangent),
                    normalize(IN.binormal),
                    normalize(IN.normal));

    vec3 bumpNormal = texture(normalTex, IN.texCoord).xyz;
    bumpNormal = normalize(TBN * normalize(bumpNormal * 2.0 - 1.0));

    return bumpNormal;
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

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//Based on observations by Disney and adopted by Epic Games, the lighting looks more correct squaring the roughness in both the geometry and normal distribution function.
//https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
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
    vec3 albedo = vec3(
        pow(texture(diffuseTex, IN.texCoord).r, 2.2),
        pow(texture(diffuseTex, IN.texCoord).g, 2.2),
        pow(texture(diffuseTex, IN.texCoord).b, 2.2));

    vec3 normal = GetNormalFromTexture();
    float metallic = texture(metallicTex, IN.texCoord).r;
    float roughness = texture(roughnessTex, IN.texCoord).r;
    float ao = texture(ambiantOccTex, IN.texCoord).r;

    vec3 F0 = vec3(0.04); //surface reflection at zero incidence  
    F0 = mix(F0, albedo, metallic);
    
     // reflectance equation
    vec3 lOut = vec3(0.0);

    for(int i = 0; i < 1 ; ++i)
    {
        //Calculate light radiance
        //vec3 wi = normalize(lightPosList[i] - IN.worldPos); //direction vector from fragmnet to lightSource
        vec3 wi = normalize(vec3(-1.0) * vec3(0.0, 1.0, 1.0));
        vec3 wo = normalize(cameraPos - IN.worldPos); //Direction vector from fragment to camera
        vec3 halfVector = (wi + wo) / length(wi + wo);

        float attenuation = CalculateInverseSquareAttenuation(lightPosList[i], IN.worldPos);
        
        vec3 radiance = lightColorList[i] * attenuation;

        // cook-torrance BRDF
        float NDF = DistributionGGX(normal, halfVector, roughness);       
        float G   = GeometrySmith(normal, wo, wi, roughness);
        vec3 F  = fresnelSchlick(max(dot(halfVector, wo), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= (1.0 - metallic);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, wo), 0.0) * max(dot(normal, wi), 0.0)  + 0.0001; //We add 0.0001 to the denominator to prevent a divide by zero in case any dot product ends up 0.0.
        vec3 specular     = numerator / denominator;

        //add to outgoing radiance lOut
        float NdotL = max(dot(normal, wi), 0.0);  
        lOut += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + lOut;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    fragColour = vec4(color, 1.0); 
}