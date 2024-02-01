#version 450 core

layout (std140) uniform Palette
{
	vec4 colours[16];
};  

layout(std430, binding = 1) buffer pixels
{
    int pixelData[];
};

layout(std430, binding = 2) buffer attributes
{
    int attributeData[];
};

const int ZX_WIDTH  = 256;
const int ZX_HEIGHT = 192;

const int ATTRIB_WIDTH  = 8;
const int ATTRIB_HEIGHT = 8;

const int ZX_ATTRIBS_WIDTH  = ZX_WIDTH / ATTRIB_WIDTH;
const int ZX_ATTRIBS_HEIGHT = ZX_HEIGHT / ATTRIB_HEIGHT;

uniform vec2 screen;

out vec4 fragColour;

void main(void)	{
	vec2 d = vec2(ZX_WIDTH, ZX_HEIGHT) / screen;

	ivec2 zxPixel = ivec2(gl_FragCoord.xy * d);

	int linearPixel = (zxPixel.y * ZX_WIDTH) + zxPixel.x;

	int realByte = linearPixel / 8;
	int bitIndex = linearPixel % 8;

	int bit = pixelData[realByte] & (1 << bitIndex);

	ivec2 zxAttrib = ivec2(zxPixel / ivec2(ATTRIB_WIDTH, ATTRIB_HEIGHT));

	int realAttrib = (zxAttrib.y * (ZX_ATTRIBS_WIDTH)) + zxAttrib.x;

	int attrib 		= attributeData[realAttrib];

	int paperColourIndex = attrib & 7;
	int inkColourIndex   = (attrib & 56) >> 3;
	int brightMode 		 = (attrib & 128) >> 7;

	inkColourIndex 	 += 8 * brightMode;
	paperColourIndex += 8 * brightMode;

	vec4 paperColour 	= colours[paperColourIndex];
	vec4 inkColour 		= colours[inkColourIndex];

	fragColour = bit > 0 ? inkColour : paperColour;
}