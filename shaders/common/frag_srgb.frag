#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

#include "image.shinc"

layout(std140, binding = 0) uniform SRGBParams
{
	Image img;
    float boost;
};


void main() 
{
	int x = int(vUV.x * img.width);
	int y = int(vUV.y * img.height);	

    vec3 rgb = read_pixel (img, x, y).rgb*boost;
    outColor = vec4(rgb, 1.0);
}

