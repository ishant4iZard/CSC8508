#version 450
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive	   : enable

#include "Data.glslh"

layout (location = 0) in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	flat int texID;
} IN;

layout (location = 0) out vec4 fragColor;

void main() {
	float shadow = 1.0; // New !

	vec3 shadowNDC      = IN.shadowProj.xyz / IN.shadowProj.w;
    vec2 sampleCoord    = vec2(shadowNDC.x, 1.0f - shadowNDC.y);
    float shadowSample 	= (texture(shadowTex, sampleCoord).r);

    float offset        = -0.001f;

    if(shadowNDC.x > 0.0f && shadowNDC.x < 1.0f && 
       shadowNDC.y > 0.0f && shadowNDC.y < 1.0f && 
       abs(shadowNDC.z) > 0.0f) {
        if(shadowSample < shadowNDC.z + offset) {
            shadow = 0.5f;
        }
    } 
	
	vec3  incident = normalize ( lightPos - IN.worldPos );
	float lambert  = max (0.0 , dot ( incident , IN.normal )) * 0.9; 
	
	vec3 viewDir = normalize ( cameraPos - IN . worldPos );
	vec3 halfDir = normalize ( incident + viewDir );

	float rFactor = max (0.0 , dot ( halfDir , IN.normal ));
	float sFactor = pow ( rFactor , 80.0 );
	 
	vec4 albedo = IN.colour;
	
	if(IN.texID >= 0) {
		albedo *= texture(textures[IN.texID], IN.texCoord);
	}
	
	albedo.rgb = pow(albedo.rgb, vec3(2.2));
	
	fragColor.rgb = albedo.rgb * 0.05f; //ambient
	
	fragColor.rgb += albedo.rgb * lightColour.rgb * lambert * shadow; //diffuse light
	
	fragColor.rgb += lightColour.rgb * sFactor * shadow; //specular light
	
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));

	fragColor.a = albedo.a;
}