#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable

#include "payload.shinc"
#include "bindings.h"

layout(location = 0) rayPayloadInNV Payload payload;
hitAttributeNV vec2 attribs;

struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;
};

struct CompVertex
{
	float a, b, c, d, e, f, g, h;
};

Vertex UnpackVertex(in CompVertex compVert)
{
	Vertex v;
	v.Position = vec3(compVert.a, compVert.b, compVert.c);
	v.Normal = vec3(compVert.d, compVert.e, compVert.f);
	v.TexCoord = vec2(compVert.g, compVert.h);
	return v;
}

layout(buffer_reference, std430, buffer_reference_align = 4) buffer VextexBuf
{
	CompVertex v;
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
	uint material; // 0: lamertian, 1: metal, 2: dielectric	
	float fuzz;
	float ref_idx;
};

layout(std430, binding = BINDING_ColoredIndexedTriangleList) buffer Params
{
	ColoredIndexedTriangleList[] coloredIndexedTriangleLists;
};

void main()
{
	ColoredIndexedTriangleList instance = coloredIndexedTriangleLists[gl_InstanceCustomIndexNV];

	IndexBuf indBuf = instance.indexBuf;
	VextexBuf vertexBuf = instance.vertexBuf;
	uvec3 ind = uvec3(indBuf[3 * gl_PrimitiveID].i,indBuf[3 * gl_PrimitiveID + 1].i, indBuf[3 * gl_PrimitiveID + 2].i);

	Vertex v0 = UnpackVertex(vertexBuf[ind.x].v);
	Vertex v1 = UnpackVertex(vertexBuf[ind.y].v);
	Vertex v2 = UnpackVertex(vertexBuf[ind.z].v);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal = v0.Normal * barycentrics.x + v1.Normal * barycentrics.y + v2.Normal * barycentrics.z;
	normal = normalize(instance.normalMat * normal);

	payload.color_dis = vec4(instance.color.xyz, gl_HitTNV);
  	payload.normal = vec4(normal, 0.0);
  	payload.material = instance.material;
  	payload.fuzz = instance.fuzz;
  	payload.ref_idx = instance.ref_idx;
}

