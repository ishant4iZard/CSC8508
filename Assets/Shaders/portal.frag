// Referenced from Shader toy "simple vortex effect 2d" @ https://www.shadertoy.com/view/Ml2GDR#
// Modified by Sameer Ahmed

#version 330 core

uniform float time;
uniform sampler2D mainTex;

out vec4 fragColour;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
    vec4 center;
} IN;

const vec2  NE                  = vec2(0.05,0.0);
const vec2  Resolution          = vec2(1280, 720); // To Do : Replace with uniform
const vec4  Colour1             = vec4(0.62, 0.85, 0.29, 1.0);
const vec4  Colour2             = vec4(0.62, 0.8, 0.5, 1.0);
const float TransitionDuration  = 5.0; 
const float Intensity           = 0.2; 
const float Radius              = 0.35;
const float Speed               = 0.05;

float height(in vec2 uv) {
    return	texture(mainTex, uv).b * 
			texture(mainTex, uv + vec2(0.0, 0.1)).b;
}

vec3 normal(in vec2 uv) {
    return normalize(vec3(height(uv + NE.xy) - height(uv - NE.xy),
                          0.0,
                          height(uv + NE.yx) - height(uv - NE.yx)));
}

void main() {
    vec2 uv = IN.texCoord - vec2(0.5);
    
    float dist = length(uv);
    float angle = atan(uv.y,uv.x);
    
    vec2 ruv = uv;
    //uv = vec2(cos(angle + dist * 3.0), dist + (IN.texCoord.y - 0.5) * 2.0 + (time * Speed));
    uv = vec2(cos(angle + dist * 3.0), dist + (time * Speed));

    float h = height(uv);
    vec3 norm = normal(uv);

    float t = cos((time / TransitionDuration) * 2) * 0.5 + 0.5;
    vec4 Colour = mix(Colour1, Colour2, t);
    float alphaEdge = 1.0 - smoothstep(0.0, Radius, length(ruv)); // Transparent around edges
    float alphaCenter = smoothstep(0.0, (Radius - Radius * 0.5), length(ruv)); // Transparent in the center

    vec4 colourEdge = mix(vec4(0.0),
                         mix(
                             mix(Colour + Colour * Intensity,
                                 texture(mainTex, uv),
                                 0.2),
                             texture(mainTex, norm.xz * 0.5 + 0.5),
                             0.3),
                         alphaEdge);

    vec4 colourCenter = mix(vec4(0.0),
                           mix(
                               mix(Colour + Colour * Intensity,
                                   texture(mainTex, uv),
                                   0.2),
                               texture(mainTex, norm.xz * 0.5 + 0.5),
                               0.3),
                           alphaCenter);

    fragColour = mix(colourEdge, colourCenter, alphaEdge);
    fragColour.a = fragColour.a + fragColour.a * 3;
}