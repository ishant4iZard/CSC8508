#version 330 core
 
out vec4 FragColor;
 
in vec2 TexCoords;
 
uniform float time;
uniform sampler2D blendTexture;
uniform sampler2D cccTexture;
const float TransitionDuration  = 0.5;
 
void main()
{
 
    vec4 col = texture(cccTexture, TexCoords);
    vec4 col2 = texture(blendTexture, TexCoords);
 
    float phase = (time / TransitionDuration) * 2.0 * 3.14159;   
    float t = cos(phase) * 0.5 + 0.5;
     col2.a = mix(col2.a, 0, t);
 
    if(col2.a > 0.2f)
	    FragColor = col2;
    else
	    FragColor = col;
}