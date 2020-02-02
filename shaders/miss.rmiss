#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable

#include "payload.shinc"

layout(location = 0) rayPayloadInNV Payload payload;

void main()
{
	vec3 direction = gl_WorldRayDirectionNV;
	float t = 0.5 * (direction.y + 1.0);
	vec3 color = (1.0 - t)*vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	payload.color_dis = vec4(color, -1.0);
}
