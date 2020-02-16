#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable

#include "payload.shinc"

layout(location = 0) rayPayloadInNV Payload payload;

layout(std140, binding = 5) uniform Params
{
	vec4 color0;
	vec4 color1;
};

void main()
{
	vec3 direction = gl_WorldRayDirectionNV;
	float t = 0.5 * (direction.y + 1.0);
	vec3 color = (1.0 - t)*color0.xyz + t * color1.xyz;
	payload.color_dis = vec4(color, -1.0);
}

