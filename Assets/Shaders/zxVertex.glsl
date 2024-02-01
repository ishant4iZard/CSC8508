#version 330 core

const vec4[] triVerts = vec4[](
	vec4(-1,-1,0,1),
	vec4(-1, 3,0,1),
	vec4( 3,-1,0,1)
);

void main(void)	{
	gl_Position	  = triVerts[gl_VertexID];
}