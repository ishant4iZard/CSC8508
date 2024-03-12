// Shader calculation inspired by "Worm-hole" from  https://www.shadertoy.com/view/4ll3zX
// Modified by Qixu Feng

#version 330 core

uniform float time;

const vec2 iResolution = vec2(1280, 720);

in Vertex{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColor;

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
	vec2 b = floor(n);
    vec2 f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

//Fractal Brownian Motion to create more complex noise
float fbm(vec2 n) {
	float total = 0.0, amplitude = 0.5;
	for (int i = 0; i <4; i++) {
        total += noise(n ) * amplitude;
		n += n;
		amplitude *= 0.5;
	}
	return total;
}

vec2 field(vec2 uv) {
    vec2 dir1 = uv - vec2(0.5, 0.5);
    vec2 dir2 = vec2(dir1.y, -dir1.x);
    float len = length(dir1);
    vec2 a0 = len > 0.7 ? dir1 / (0.1 + (len - 0.3) * 15) : dir2 * .8;
    return a0 * 3.0 + dir2 * len * 0.9;
}

float getColor(vec2 uv) { return fbm(uv * 3.0); }

vec2 calculateNext(vec2 uv, float t) {
    vec2 k1 = field(uv);
    vec2 k2 = field(uv + k1 * t / 2);
    vec2 k3 = field(uv + k2 * t / 2);
    vec2 k4 = field(uv + k3 * t);
       
    return uv + t / 3.0 * (k1 + 2.0 * k2 + 2.0 * k2 + k3);
}

vec4 getColor(vec2 uv, float cf, float per) {
    float t1 = per * cf;
    float t2 = t1 + per;
    
    float k1 = 0.3;
    float k2 = 0.3;
    
    vec2 uv1 = calculateNext(uv, t1 * k1 + k2);
    vec2 uv2 = calculateNext(uv, t2 * k1 + k2);
    
    float c1 = getColor(uv1);
    float c2 = getColor(uv2);
    
    return vec4(mix(c2, c1, cf));
}

void main(void) {
    vec2 uv = IN.texCoord;

    float per = 2.0;
    float colorfract = fract(time / per);
    float len = length(field(uv));

    vec4 color = getColor(uv, colorfract, per);
    
    color =  1.0 - (abs(color - 0.5) * 5.0);
    color *= 0.5 * vec4(0.3, 0.6, 1.1, 1.0) * pow(len, -0.3);
    color += 0.25 * vec4(1.0, 0.6, 0.2, 1.0) * pow(abs(len - 0.35), -0.7);
    color *= 0.8;

    float edgeThickness = 0.1;
    float alpha = smoothstep(0.0, edgeThickness, len);
    //color.a = alpha;  //mix(1.0, 0.0, alpha);

    fragColor = color;
}