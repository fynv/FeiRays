#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_tracing : enable

#include "../common/payload.shinc"
#include "../common/bindings.h"

layout(location = 0) rayPayloadInEXT Payload payload;
hitAttributeEXT vec2 attribs;

struct CompVec3
{
	float a, b, c;
};

vec3 UnpackVec3(in CompVec3 compv)
{
	return vec3(compv.a, compv.b, compv.c);
}

layout(buffer_reference, std430, buffer_reference_align = 4) buffer VextexBuf
{
	CompVec3 v;
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer IndexBuf
{
	uint i;
};

struct ColoredIndexedTriangleList
{
	mat3 normalMat;
	vec4 color;
	VextexBuf vertexBuf;
	IndexBuf indexBuf;
	uint type; // 0: lamertian, 1: metal, 2: dielectric, 3: emissive, 4: foggy
	float fuzz;
	float ref_idx;
	float density;
};

layout(std430, binding = BINDING_ColoredIndexedTriangleList) buffer Params
{
	ColoredIndexedTriangleList[] coloredIndexedTriangleLists;
};

const uint cPresets[5] = {
	MAT_OPAQUE_BIT | MAT_DIFFUSE_BIT,
	MAT_OPAQUE_BIT | MAT_SPECULAR_BIT,
	MAT_FRESNEL_BIT | MAT_SCATTER_BIT,
	MAT_OPAQUE_BIT | MAT_EMIT_BIT,
	MAT_SCATTER_BIT
};

void main()
{
	ColoredIndexedTriangleList instance = coloredIndexedTriangleLists[gl_InstanceCustomIndexEXT];

	uint i0 = instance.indexBuf[3 * gl_PrimitiveID].i;
	uint i1 = instance.indexBuf[3 * gl_PrimitiveID + 1].i;
	uint i2 = instance.indexBuf[3 * gl_PrimitiveID + 2].i;

	vec3 norm0 = UnpackVec3(instance.vertexBuf[i0].v);
	vec3 norm1 = UnpackVec3(instance.vertexBuf[i1].v);
	vec3 norm2 = UnpackVec3(instance.vertexBuf[i2].v);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal = norm0 * barycentrics.x + norm1 * barycentrics.y + norm2 * barycentrics.z;
	normal = normalize(instance.normalMat * normal);

	payload.material_bits = cPresets[instance.type];
	payload.t = gl_HitTEXT;
	payload.color0 = instance.color.xyz;
	payload.color1 = instance.color.xyz;
	payload.color2 = instance.color.xyz;
	payload.normal = normal;  	
  	payload.f1 = instance.fuzz;
  	payload.f2 = instance.density;
	
	if (instance.type == 0) payload.f0 = 0.0;
	if (instance.type == 1) payload.f0 = 1.0;
	if (instance.type == 2) payload.f0 = instance.ref_idx;
}

