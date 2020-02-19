#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "payload.shinc"

layout(location = 0) rayPayloadInNV Payload payload;

layout(binding = 4) uniform samplerCube[] cubeSamplers;

layout(std140, binding = 5) uniform Params
{
	mat3 transform;
    int tex_idx;
};


void main()
{
	vec3 direction = transform*gl_WorldRayDirectionNV;
	vec3 color = texture(cubeSamplers[tex_idx], direction).xyz;

	payload.t = -1.0;
	payload.material_bits = MAT_OPAQUE_BIT | MAT_EMIT_BIT;
	payload.color0 = color;
}


