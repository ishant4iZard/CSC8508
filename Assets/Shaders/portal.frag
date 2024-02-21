#version 330 core

uniform float time;
uniform sampler2D mainTex;

out vec4 fragColour;

float height(in vec2 uv) {
    return	texture(mainTex, uv).b * 
			texture(mainTex, uv + vec2(0.0, 0.1)).b;
}

const vec2 NE = vec2(0.05,0.0);

vec3 normal(in vec2 uv) {
    return normalize(vec3(height(uv + NE.xy) - height(uv - NE.xy),
                          0.0,
                          height(uv + NE.yx) - height(uv - NE.yx)));
}

const vec4 Color = vec4(0.82,0.87,0.19,1.0);
const float Intensity = 0.2; 
const float Speed = 0.02;
const vec2 Resolution = vec2(1280, 720); // Replace with uniform

void main() {
	vec2 uv = gl_FragCoord.xy/Resolution.xy-vec2(.5);
    uv.y *= Resolution.y/Resolution.x;
    
    float dist = length(uv);
    float angle = atan(uv.y,uv.x);
    
    vec2 ruv = uv;
    uv = vec2(cos(angle + dist * 3.0), dist + (time * Speed));

    float h = height(uv);
    vec3 norm = normal(uv);

	fragColour = 
        mix( vec4(0.), 
            mix(
                mix(Color + Color * Intensity,
                texture(mainTex,uv),0.2),
            texture(mainTex,norm.xz*0.5+0.5),0.3),min(1.,length(ruv)*10.));
}