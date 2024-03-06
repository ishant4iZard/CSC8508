// Blackhole calculation inspired by "Yet Another Black Hole " from https://www.shadertoy.com/view/Xtcczs
// Modified by Qixu Feng

#version 330 core

uniform sampler2D blackholeTex;
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

mat2 rotateBlackhole(float speed, float radius) {
	float rotateSpeed	= speed * pow(radius, 8.0f);
	float sinRSpeed		= sin(rotateSpeed);
	float cosRSpeed		= cos(rotateSpeed);
	
	return mat2(vec2(cosRSpeed, -sinRSpeed), vec2(sinRSpeed, cosRSpeed));
}

void main(void) {
	//vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 uv = IN.texCoord;
	vec2 center = (iResolution.xy * 0.5f) / iResolution.xy;
	//vec2 center = vec2(0.5f);
	vec2 dir = normalize(uv - center);
	vec3 blackholeColor = vec3(0.8f, 0.4f, 0.2f);

	uv.y *= (iResolution.y / iResolution.x);
	center.y *= (iResolution.y / iResolution.x);

	float distUC = distance(uv, center);
	float _s = 0.15f;
	float _a = 0.005f;
	vec2 uvp = uv + min(
		texture(blackholeTex, _s * vec2(uv.x + time * 0.1f, uv.y + time * -0.3f)).r,
		texture(blackholeTex, _s * vec2(uv.x + time * -0.3f, uv.y + time * 0.2f)).r)
		* _a * pow(1.0f - distUC, 5.0f);

	float distUVPC = distance(uvp, center);
	float idistUVPC = 1.0 - distUVPC;

	float ft = step(0.06f, distUVPC);
	ft *= smoothstep(0.065f, 0.08f, distUVPC);
	ft = ft * (1.0 - distUVPC * 1.6) * 4.0f;
	float front = ft;

	float speed = -0.06f;
	float phase1 = fract(time * speed + 0.5f);
	float phase2 = fract(time * speed);
	vec2 uv1 = (dir * pow(idistUVPC, 2.3f) * 0.2f) + phase1 * dir;
	vec2 uv2 = (dir * pow(idistUVPC, 2.3f) * 0.2f) + phase2 * dir;

	float lerp = abs((0.5f - phase1) / 0.5f);
	float sampling1 = texture(blackholeTex, uv1 * rotateBlackhole(2.4f, idistUVPC)).r;
	float sampling2 = texture(blackholeTex, uv2 * rotateBlackhole(2.4f, idistUVPC)).r;
	float sampling3 = texture(blackholeTex, uv1 * 2.0f * rotateBlackhole(2.6f, idistUVPC)).g;
	float sampling4 = texture(blackholeTex, uv2 * 2.0f * rotateBlackhole(2.6f, idistUVPC)).g;
	float sampling5 = texture(blackholeTex, uv1 * 2.0f * rotateBlackhole(4.6f, idistUVPC) * 0.4f).r;
	float sampling6 = texture(blackholeTex, uv2 * 2.0f * rotateBlackhole(4.6f, idistUVPC) * 0.6f).r;

	float stars = (1.0f - smoothstep(0.22f, 0.34f, mix(sampling3, sampling4, lerp))) * 0.4f;
	vec3 sp = mix(sampling1, sampling2, lerp) * vec3(1.0f);
	sp *= 0.2f;
	sp += stars * vec3(0.13f, 0.31f, 0.46f);
	sp += smoothstep(0.26f, 0.14f, mix(sampling5, sampling6, lerp)) * vec3(0.7f) *
		pow(idistUVPC, 8.0f) * blackholeColor;

	vec3 finalSampler = sp;
	vec3 col = blackholeColor * pow(1.0 - distUVPC, 7.0f);
	float ring = pow(smoothstep(0.32f, 0.08f, distUVPC * 1.5f) * 4.0f, 2.8f) + 1.0f;

	fragColor = vec4(finalSampler * front + 1.2f * ring * front * finalSampler * col, 1.0f);
}