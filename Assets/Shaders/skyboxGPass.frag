#version 330 core

uniform samplerCube cubeTex;

in Vertex {
	vec3 viewDir;
} IN;

out vec4 fragColour[5];

void main(void)	{
	vec4 samp = texture(cubeTex,normalize(IN.viewDir));
	fragColour[0] = pow(samp, vec4(2.2f));
}