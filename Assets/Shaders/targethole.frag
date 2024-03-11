// Shader calculation inspired by "Attracting funnel" from https://www.shadertoy.com/view/3tXSRS
// Modified by Qixu Feng

/*
#version 330 core

#define HOLE_OBJ 1
#define bump

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

struct Light
{
	vec3 position;
	vec3 color;
	float intensity;
	float attenuation;
};
Light lights[3];

struct RenderData
{
	vec3 position;
	vec3 color;
	vec3 normal;
};

vec2 opU(vec2 d1, vec2 d2) {
	return (d1.x < d2.x) ? d1 : d2;
}

vec2 rotateVec(vec2 vect, float angle) {
	vec2 rv;
	rv.x = vect.x * cos(angle) + vect.y * sin(angle);
	rv.y = vect.x * sin(angle) - vect.y * cos(angle);
	return rv;
}

//Generate pseudo-random numbers
float hash(vec3 p) {
	p = fract(p * 0.3183099 + 0.1);
	p *= 17.0;
	return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

//Generate polar noise
float polarNoise0(vec3 x) {   
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
	
    return mix(mix(mix( hash(p + vec3(0, 0, 0)), 
                        hash(p + vec3(1, 0, 0)), f.x),
                   mix( hash(p + vec3(0, 1, 0)), 
                        hash(p + vec3(1, 1, 0)), f.x), f.y),
               mix(mix( hash(p + vec3(0, 0, 1)), 
                        hash(p + vec3(1, 0, 1)), f.x),
                   mix( hash(p + vec3(0, 1, 1)), 
                        hash(p + vec3(1, 1, 1)), f.x), f.y), f.z);
}

const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float polarNoise(vec3 pos) {
    vec3 q = 8.0 * pos;
    float f = 0.0;
    f  = 0.5000 * polarNoise0(q); q = m * q * 2.0;
    f += 0.2500 * polarNoise0(q); q = m * q * 2.0;
    f += 0.1250 * polarNoise0(q); q = m * q * 2.0;
    f += 0.0625 * polarNoise0(q); q = m * q * 2.0;
    
    return f;
}

//float time2;
const float pati = 0.33;
const float txtSpeed = 1.7;
float polarNoise2(vec3 pos) {
    float a = 2.0 * atan(pos.y, pos.x);
    vec3 pos1 = vec3(pos.z, length(pos.yx) + time * txtSpeed, a);
    vec3 pos2 = vec3(pos.z, length(pos.yx) + time * txtSpeed, a + 12.57);    
    
    float f1 = polarNoise(pos1);
    float f2 = polarNoise(pos2);
    float f = mix(f1, f2, smoothstep(-5.0, -6.285, a));
    
    f = smoothstep(0.01, 0.2, f) - smoothstep(0.2, 0.52, f) + smoothstep(0.45, 0.63, f);
    f = 0.8 - pati + f * pati;
    
    return f;
}

float polarNoiseN(vec3 pos) {
    vec3 q = 8.0 * pos;
    float f = 0.0;
    f  = 0.5000 * polarNoise0(q); q = m * q * 2.0;
    f += 0.2500 * polarNoise0(q); q = m * q * 2.0;
    
    return f;
}

float polarNoise2N(vec3 pos) {
    float a = 2.0 * atan(pos.y, pos.x);
    vec3 pos1 = vec3(pos.z, length(pos.yx) + time * txtSpeed, a);
    vec3 pos2 = vec3(pos.z, length(pos.yx) + time * txtSpeed, a + 12.57);    
    
    float f1 = polarNoiseN(pos1);
    float f2 = polarNoiseN(pos2);
    float f = mix(f1, f2, smoothstep(-5., -6.285, a));

    return f;
}

//Calculate hole shape
const float bottom = 7.5;
float holeFunct(vec2 pos, float holeWidth) {
    float f1 = min(0.6 / (max(length(pos) - holeWidth, 0.0)), bottom);
    float f2 = 4.0 * min(0.6 / (max(length(pos) + holeWidth * 0.5, 0.0)), bottom);
    
    //return min(f1, f2);
    return f1;
}

const float bumpHeight = 0.015;
const float holeWidth = 0.5;
float map_hole(vec3 pos, bool hasBump) {  
    #ifdef bump
    if (hasBump) {
       vec3 pos2 = vec3(pos.yx, 0);
       pos.z += bumpHeight * polarNoise2N(pos2);
    }
    #endif     
    float d = pos.z + holeFunct(pos.yx, holeWidth);
    
    return d;
}

vec2 map(vec3 pos, bool hasBump) {
    vec2 res;
    float hole = map_hole(pos, hasBump);

    res = vec2(hole, HOLE_OBJ);
    return res;
}

float trace(vec3 cam, vec3 ray, float maxdist)  {
    float t = 1.8;
    float dist;
    vec3 pos;
    
  	for (int i = 0; i < 70; ++i) {
    	pos = ray * t + cam;
    	vec2 res = map(pos, false);
        dist = res.x;

        if (dist > maxdist || abs(dist) < 0.0001) break;

        t += dist * (0.85 - float(i) * 0.0122);
  	}
        
  	return t;
}

vec3 getNormal(vec3 pos, float e) {
    vec2 q = vec2(0, e);
    vec3 normal = normalize(vec3(map(pos + q.yxx, true).x - map(pos - q.yxx, true).x,
                          map(pos + q.xyx, true).x - map(pos - q.xyx, true).x,
                          map(pos + q.xxy, true).x - map(pos - q.xxy.x, true)));
    return normal;
}

const float cdif = 0.012;
const float maxDist = 60;
vec3 objectColor(vec3 norm, vec3 pos) {
    vec3 color;
    vec3 posr = vec3(pos.yx, cdif);
    vec3 posg = vec3(pos.yx, 0);
    vec3 posb = vec3(pos.yx, -cdif);

    color = vec3(polarNoise2(posr), polarNoise2(posg), polarNoise2(posb));
    color *= smoothstep(bottom, bottom * 0.25, -pos.z);
    color *= smoothstep(maxDist * 0.3, maxDist * 0.1, length(pos.yx));

    return color;
}

float softShadow(vec3 ro, vec3 rd, float mint, float tmax) {
	float res = 1.0;
    float t = mint;
    for(int i = 0; i < 25; i++) {
    	float h = map(ro + rd * t, false).x;
        res = min(res, 4.5 * h / t);
        t += clamp(h, 0.01, 0.12);
        if(h < 0.001 || t > tmax) break;
    }

    return smoothstep(0.0, 0.8, res);
}

const float specint_hole = 0.047;
const float specshin_hole = 23.0;
const float shi = 0.84;
const float shf = 0.4;
const vec3 specColor = vec3(0.95, 0.97, 1.);
vec3 cameraPos = vec3(0.5, -0.4, 10);
vec3 lightShading(Light lights, vec3 norm, vec3 pos, vec3 ocol) {
    vec3 pl = normalize(lights.position - pos);
    float dlp = distance(lights.position, pos);
    vec3 pli = pl / pow(1.0 + lights.attenuation * dlp, 2.0);
    vec3 nlcol = normalize(lights.color);
    float dnp = dot(norm, pli);
    
    //Diffuse shading
    vec3 col = ocol * nlcol * lights.intensity * smoothstep(-0.1, 1.0, dnp);
    
    //Specular shading
    if (dot(norm, lights.position - pos) > 0.0) {
        col += specColor * nlcol * lights.intensity * specint_hole * 
            pow(max(0.0, dot(reflect(pl, norm), normalize(pos - cameraPos))),
            specshin_hole) * smoothstep(bottom, 0, -pos.z);
    }
    col *= shi * softShadow(pos, normalize(lights.position - pos), shf, 6.0) + 1.0 - shi;

    return col;
}

vec3 allLightShading(vec3 norm, vec3 pos, vec3 ocol) {
    vec3 col = vec3(0.0);
    for (int i = 0; i < 3; i++) {
        col += lightShading(lights[i], norm, pos, ocol);
    }
    return col;
}

float fov = 5.8;
vec3 GetCameraRayDir(vec2 vWindow, vec3 vCameraDir, float fov) {
	vec3 vForward = normalize(vCameraDir);
	vec3 vRight = normalize(cross(vec3(0.0, 1.0, 0.0), vForward));
	vec3 vUp = normalize(cross(vForward, vRight));
	vec3 vDir = normalize(vWindow.x * vRight + vWindow.y * vUp +
        vForward * fov);

	return vDir;
}


vec3 cameraTarget = vec3(0, 0, 0);
vec3 cameraDir;
void setCamera() {
   cameraPos = vec3(0.0, 0.0, 10.0);
   cameraTarget = vec3(0.0, 0.0, 0.0);
   cameraDir = cameraTarget - cameraPos;   
}


const vec3 ambientColor = vec3(0.3);
const float normalD = 0.0001;
const float ambientint = 0.12;
RenderData trace0(vec3 tpos, vec3 ray) {
  float t = trace(tpos, ray, maxDist);
  vec3 col;
    
  vec3 pos = tpos + t * ray;
  vec3 norm;
  if (t < maxDist * 0.65) {
      norm = getNormal(pos, normalD);
      col = objectColor(norm, pos);
      col = ambientColor * ambientint + allLightShading(norm, pos, col);
  }
  return RenderData(col, pos, norm);
}

float fresnel(vec3 ray, vec3 norm, float n2) {
   float n1 = 1.0;
   float angle = acos(-dot(ray, norm));
   float r0 = dot((n1 - n2) / (n1 + n2), (n1 - n2) / (n1 + n2));
   float r = r0 + (1.0 - r0) * pow(1.0 - cos(angle), 5.0);
   return clamp(r, 0.0, 0.8);
}

vec4 render(vec2 fragCoord) {
  lights[0] = Light(vec3(7.0, -8.0, 3.5), vec3(1.0, 1.0, 1.0), 11.2, 0.1);
  lights[1] = Light(vec3(-14.0, 2.5, 29.0), vec3(0.7, 0.82, 1.0), 7.0, 0.1);
  lights[2] = Light(vec3(1.0, 0.0, 1.2), vec3(1.0, 0.6, 0.4), 0.6, 0.1);
    
  vec2 uv = gl_FragCoord.xy / iResolution.xy; 
  //vec2 uv = IN.texCoord;
  uv = uv * 2.0 - 1.0;
  uv.x *= iResolution.x / iResolution.y;

  vec3 ray = GetCameraRayDir(uv, cameraDir, fov);
    
  RenderData traceinf = trace0(cameraPos, ray);
  vec3 color = traceinf.color;
    
  return vec4(color, 1.0);
}

const int aasamples = 1;
const float aawidth = 0.8;
void main(void) {
    setCamera();

    vec4 orv;
    vec4 vs = vec4(0.0);
    for (int j = 0; j < aasamples ;j++) {
       float oy = float(j) * aawidth / max(float(aasamples - 1), 1.0);
       for (int i = 0; i < aasamples; i++) {
          float ox = float(i) * aawidth / max(float(aasamples - 1), 1.0);
          vec4 rv = render(IN.texCoord + vec2(ox, oy));
          vs += rv;        
       }
    }
    fragColor = vs / vec4(aasamples * aasamples); 
}
*/

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
	vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

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
    vec2 dir = uv - vec2(0.5, 0.5);
    vec2 dir2 = vec2(dir.y, -dir.x);
    float l = length(dir);
    vec2 a0 = l > 0.3 ? dir/(0.1 + (l - 0.3) * 20.) : dir2 * 5.;
    return a0 * 3. + dir2 * l * 0.9;
}

float getColor(vec2 uv) {
    return fbm(uv * 5.);   
}

vec2 calcNext(vec2 uv, float t) {
    vec2 k1 = field(uv);
    vec2 k2 = field(uv + k1*t/2.);
    vec2 k3 = field(uv + k2*t/2.);
    vec2 k4 = field(uv + k3*t);
       
    return uv + t/6.*(k1+2.*k2+2.*k2+k3);
}

vec4 getColor(vec2 uv, float cf, float per) {
    float t1 = per * cf;
    float t2 = t1 + per;
    
    float k1 = 0.4;
    float k2 = 0.4;
    
    vec2 uv1 = calcNext(uv, t1 * k1 + k2);
    vec2 uv2 = calcNext(uv, t2 * k1 + k2);
    
    float c1 = getColor(uv1);
    float c2 = getColor(uv2);
    
    return vec4(mix(c2, c1, cf));
}

void main(void) {
    //vec2 uv = (fragCoord.xy + (iResolution.yx - iResolution.xx)/2.)/iResolution.y;
    vec2 uv = IN.texCoord;
    //uv.y *= (iResolution.y / iResolution.x);
    float per = 2.;
    
    float cf = fract(time / per);
    vec4 c = getColor(uv,cf, per);
    float l = length(field(uv));
    c =  1. - (abs(c-0.5)*5.);
    
    
    // some empirical coefficients
    c *= 0.5*vec4(0.3, 0.6, 1.1, 1.)*pow(l,-0.3);
    c += 0.25*vec4(1.0, 0.6, 0.2, 1.)*pow(abs(l-0.35),-0.7);
    c*=0.8;

	fragColor = c;
}