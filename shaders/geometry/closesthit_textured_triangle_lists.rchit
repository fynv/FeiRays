#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "../common/payload.shinc"
#include "../common/bindings.h"

layout(location = 0) rayPayloadInEXT Payload payload;
hitAttributeEXT vec2 attribs;

layout(binding = 3) uniform sampler2D[] textureSamplers;

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

struct Material
{
	vec3 diffuse;
	int textureId;
};

struct CompMaterial
{
	float a, b, c;
	int d;
};

Material UnpackMaterial(in CompMaterial compMat)
{
	Material m;
	m.diffuse = vec3(compMat.a, compMat.b, compMat.c);
	m.textureId = compMat.d;
	return m;
}

layout(buffer_reference, std430, buffer_reference_align = 4) buffer VextexBuf
{
	CompVertex v;
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer MaterialBuf
{
	CompMaterial m;
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer MaterialIndexBuf
{
	int i;
};

struct TexturedTriangleList
{
	mat3 normalMat;
	VextexBuf vertexBuf;
	MaterialBuf materialBuf;
	MaterialIndexBuf materialIdxBuf;
};

layout(std430, binding = BINDING_TexturedTriangleList) buffer Params
{
	TexturedTriangleList[] texturedTriangleList;
};


void main()
{
	TexturedTriangleList instance = texturedTriangleList[gl_InstanceCustomIndexEXT];

	Vertex v0 = UnpackVertex(instance.vertexBuf[3 * gl_PrimitiveID].v);
	Vertex v1 = UnpackVertex(instance.vertexBuf[3 * gl_PrimitiveID + 1].v);
	Vertex v2 = UnpackVertex(instance.vertexBuf[3 * gl_PrimitiveID + 2].v);

	Material mat = UnpackMaterial(instance.materialBuf[instance.materialIdxBuf[gl_PrimitiveID].i].m);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal = v0.Normal * barycentrics.x + v1.Normal * barycentrics.y + v2.Normal * barycentrics.z;
	normal = normalize(instance.normalMat * normal);

	vec3 c = mat.diffuse;
	if (mat.textureId >= 0)
	{
		vec2 texCoord = v0.TexCoord * barycentrics.x + v1.TexCoord * barycentrics.y + v2.TexCoord * barycentrics.z;
		c*= texture(textureSamplers[mat.textureId], texCoord).xyz;
	}

	payload.t = gl_HitTEXT;
	payload.material_bits = MAT_OPAQUE_BIT | MAT_DIFFUSE_BIT;
	payload.color1 = c;
	payload.f0 = 0.0;
	payload.normal = normal;
}




