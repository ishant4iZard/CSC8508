#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrix;


layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;
layout(location = 5) in vec4 jointWeights;
layout(location = 6) in ivec4 jointIndices;

uniform mat4 joints[128];

out Vertex
{
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
    vec3 tangent;
    vec3 binormal;
	vec3 worldPos;
} OUT;

void main(void){
	vec4 localPos = vec4(position,1.0f);
	vec4 skelPos = vec4(0,0,0,0);

	for(int i=0;i<4;i++){
		int jointIndex = jointIndices[i];
		float jointWeight = jointWeights[i];
		skelPos +=joints[jointIndex] * localPos * jointWeight;
	}

	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	mat3 normalMatrix = transpose(inverse(mat3( modelMatrix )));
	OUT.normal = normalize ( normalMatrix * normalize ( normal )); //World normal
    OUT.tangent = normalize(normalMatrix * normalize(tangent.xyz)); //World tangent
    OUT.binormal = cross(OUT.tangent, OUT.normal) * tangent.w;
	
    OUT.shadowProj 	=  shadowMatrix * vec4 (position,1);
	OUT.worldPos 	= (modelMatrix * vec4 (position ,1)). xyz ;
		
	OUT.texCoord	= texCoord;
	gl_Position = mvp * vec4(skelPos.xyz , 1.0);
}
