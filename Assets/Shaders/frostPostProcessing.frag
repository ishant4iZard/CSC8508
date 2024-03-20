#version 330 core
 
out vec4 FragColor;
 
in vec2 TexCoords;
 
uniform float time;
uniform int isActive;
uniform sampler2D blendTexture;
uniform sampler2D sceneTexture;



uniform int powerupType; 
// 1 : ice
// 2 : wind
// 3 : sand
const float TransitionDuration  = 2.5;
 
void main()
{
 
    vec4 col = texture(sceneTexture, TexCoords);
    vec4 col2 = texture(blendTexture, TexCoords);

    //change the color
     vec4 iceTint = vec4(0.54, 0.97, 1.0, 1.0); 
     vec4 sandTint = vec4(1.00, 0.88, 0.01, 1.0); 
     vec4 windTint = vec4(0.0, 0.6, 0.90, 1.0); 


    vec4 finalColor = col2 ;


    float phase = (time / TransitionDuration) * 2.0 * 3.14159;   
    float t = cos(phase) * 0.5 + 0.5;
    col2.a = mix(col2.a, 0, t);
 
    float buffer = 0;
   
    if(powerupType == 1){
        finalColor = col2*iceTint;
        buffer = 0.40f;
    if(isActive > 0 && col2.a > buffer)
 	   FragColor =  finalColor ;        
    else
	    FragColor = col; 
     }

    else if(powerupType == 2){
        finalColor = col2*windTint;
        buffer = 0.25f;
    if(isActive > 0 && col2.a > buffer)
 	     FragColor = finalColor;        
    else
	    FragColor = col; 
     }

    else if(powerupType == 3){
        finalColor = col2*sandTint;
        buffer = 0.2f;
    if(isActive > 0 && col2.a > buffer)
 	     FragColor = finalColor;        
    else
	    FragColor = col; 
    }

    else 
        FragColor = col; 
}