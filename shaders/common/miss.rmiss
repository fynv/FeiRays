#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_tracing  : enable

#include "payload.shinc"
#include "sunlight.shinc"

layout(location = 0) rayPayloadInEXT Payload payload;

layout(std140, binding = 5) uniform Params
{
	vec4 color0;
	vec4 color1;
};

void main()
{
	payload.t = -1.0;
	payload.material_bits = MAT_OPAQUE_BIT | MAT_EMIT_BIT;

	vec3 color;
	int id;
	if (test_sunlights(gl_WorldRayDirectionEXT, color, id))
	{
		payload.material_bits |= MAT_LIGHT_SOURCE_BIT;
		payload.f0 = intBitsToFloat(id);
	}
	else
	{
		vec3 direction = gl_WorldRayDirectionEXT;
		float t = 0.5 * (direction.y + 1.0);
		color = (1.0 - t)*color0.xyz + t * color1.xyz;
	}
	
	payload.color0 = color;
}

