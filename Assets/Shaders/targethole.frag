// Shader calculation inspired by "Worm-hole" from  https://www.shadertoy.com/view/4ll3zX
// Modified by Qixu Feng

#version 330 core

uniform float time;
uniform sampler2D mainTex;

const vec2 iResolution = vec2(1280, 720);
const vec2  NE = vec2(0.05, 0.0);
const float Radius = 0.57;
const float Intensity = 0.15; 

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
	float total = 0.0, amplitude = 0.8;
	for (int i = 0; i < 4; i++) {
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
    vec2 a0 = len > 0.7 ? dir1 / (0.1 + (len - 0.3) * 15) : dir2 * 0.2;
    return a0 * 3.0 + dir2 * len * 0.9;
}

float getColor(vec2 uv) { return fbm(uv * 3.0); }

vec2 calculateNext(vec2 uv, float t) {
    vec2 k1 = field(uv);
    vec2 k2 = field(uv + k1 * t / 2);
    vec2 k3 = field(uv + k2 * t / 2);
    vec2 k4 = field(uv + k3 * t);
       
    return uv + t / 3.0 * (k1 + 20.0 * k2 + 20.0 * k2 + k3);
}

vec4 getColor(vec2 uv, float cf, float per) {
    float t1 = per * cf;
    float t2 = t1 + per;
    
    float k1 = 0.9;
    float k2 = 0.1;
    
    vec2 uv1 = calculateNext(uv, t1 * k1 + k2);
    vec2 uv2 = calculateNext(uv, t2 * k1 + k2);
    
    float c1 = getColor(uv1);
    float c2 = getColor(uv2);
    
    return vec4(mix(c2, c1, cf));
}

float height(in vec2 uv) {
    return	texture(mainTex, uv).b * 
			texture(mainTex, uv + vec2(0.0, 0.1)).b;
}

vec3 normal(in vec2 uv) {
    return normalize(vec3(height(uv + NE.xy) - height(uv - NE.xy),
                          0.0,
                          height(uv + NE.yx) - height(uv - NE.yx)));
}

void main(void) {
    vec2 uv = IN.texCoord;// - vec2(0.5);
    


    float per = 2.0;
    float colorfract = fract(time / per);
    float len = length(field(uv));

    vec4 color = getColor(uv, colorfract, per);
    
    color =  1.0 - (abs(color - 0.5) * 5.0);
    color *= 0.5 * vec4(0.3, 0.6, 1.1, 1.0) * pow(len, -0.3);
    color += 0.25 * vec4(1.0, 0.6, 0.2, 1.0) * pow(abs(len - 0.35), -0.7);
    color *= 0.8;

    vec3 norm = normal(uv);
    float alphaEdge = 1.0 - smoothstep(0.0, Radius, len);
    float alphaCenter = smoothstep(0.0, (Radius - Radius * 0.7), len);

    vec4 colorEdge = mix(vec4(0.0), mix( mix(color - color * Intensity, texture(
        mainTex, uv), 0.2), texture(mainTex, norm.xz * 0.5 + 0.5), 0.3), alphaEdge);

    vec4 colorCenter = mix(vec4(0.0), mix( mix(color - color * Intensity, texture(
        mainTex, uv), 0.2), texture(mainTex, norm.xz * 0.5 + 0.5), 0.3), alphaCenter);

    fragColor = mix(colorEdge, colorCenter, alphaEdge);
    fragColor.a = fragColor.a + fragColor.a * 0.9;
    //fragColor = color;
}