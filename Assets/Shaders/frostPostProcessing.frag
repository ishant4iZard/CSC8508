#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D cccTexture;

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
	//FragColor = vec4(1.0, 0.0,0.0, 1.0);
} 