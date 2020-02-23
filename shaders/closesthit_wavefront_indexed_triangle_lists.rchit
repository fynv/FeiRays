#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "payload.shinc"
#include "bindings.h"

layout(location = 0) rayPayloadInNV Payload payload;
hitAttributeNV vec2 attribs;

layout(binding = 3) uniform sampler2D[] textureSamplers;

struct CompVec3
{
	float a, b, c;
};

vec3 UnpackVec3(in CompVec3 compv)
{
	return vec3(compv.a, compv.b, compv.c);
}

struct CompVec2
{
	float a, b;
};

vec2 UnpackVec2(in CompVec2 compv)
{
	return vec2(compv.a, compv.b);
}

struct Material
{
	vec3 diffuse;
	vec3 specular;
	vec3 emission;
	float shininess;
	int texId_diffuse;
	int texId_specular;
	int texId_emission;
	int mask;
};

struct CompMaterial
{
	float a, b, c, d, e, f, g, h, i, j;
	int k, l, m, n;
};

Material UnpackMaterial(in CompMaterial compMat)
{
	Material m;
	m.diffuse = vec3(compMat.a, compMat.b, compMat.c);
	m.specular = vec3(compMat.d, compMat.e, compMat.f);
	m.emission = vec3(compMat.g, compMat.h, compMat.i);
	m.shininess = compMat.j;
	m.texId_diffuse = compMat.k;
	m.texId_specular = compMat.l;
	m.texId_emission = compMat.m;
	m.mask = compMat.n;	
	return m;
}

struct Index
{
	int vertex_index;
	int normal_index;
	int texcoord_index;
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer Vec3Buf
{
	CompVec3 v;
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer Vec2Buf
{
	CompVec2 v;
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer IndexBuf
{
	Index i;
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer MaterialBuf
{
	CompMaterial m;
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer MaterialIndexBuf
{
	int i;
};

struct WavefrontIndexedTriangleList
{
	mat3 normalMat;
	Vec3Buf normalBuf;
	Vec2Buf texcoordBuf;
	IndexBuf indexBuf;
	MaterialBuf materialBuf;
	MaterialIndexBuf materialIdxBuf;
};

layout(std430, binding = BINDING_WavefrontIndexedTriangleList) buffer Params
{
	WavefrontIndexedTriangleList[] wavefrontIndexedTriangleList;
};


void main()
{
	WavefrontIndexedTriangleList instance = wavefrontIndexedTriangleList[gl_InstanceCustomIndexNV];

	Index i0 = instance.indexBuf[3 * gl_PrimitiveID].i;
	Index i1 = instance.indexBuf[3 * gl_PrimitiveID + 1].i;
	Index i2 = instance.indexBuf[3 * gl_PrimitiveID + 2].i;

	Material mat = UnpackMaterial(instance.materialBuf[instance.materialIdxBuf[gl_PrimitiveID].i].m);

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal;
	{
		vec3 norm0 = UnpackVec3(instance.normalBuf[i0.normal_index].v);
		vec3 norm1 = UnpackVec3(instance.normalBuf[i1.normal_index].v);
		vec3 norm2 = UnpackVec3(instance.normalBuf[i2.normal_index].v);
		normal = norm0 * barycentrics.x + norm1 * barycentrics.y + norm2 * barycentrics.z;
		normal = normalize(instance.normalMat * normal);
	}

	vec2 texCoord;

	if (i0.texcoord_index>=0)
	{
		vec2 texCoord0 = UnpackVec2(instance.texcoordBuf[i0.texcoord_index].v);
		vec2 texCoord1 = UnpackVec2(instance.texcoordBuf[i1.texcoord_index].v);
		vec2 texCoord2 = UnpackVec2(instance.texcoordBuf[i2.texcoord_index].v);
		texCoord = texCoord0 * barycentrics.x + texCoord1 * barycentrics.y + texCoord2 * barycentrics.z;
	}

	uint bits = MAT_OPAQUE_BIT | MAT_DIFFUSE_BIT;

	vec3 diffuse = mat.diffuse;
	if (mat.texId_diffuse >= 0)
	{
		diffuse *= texture(textureSamplers[mat.texId_diffuse], texCoord).xyz;
	}

	vec3 specular = mat.specular;
	float k = 0.0;
	if ( (mat.mask&2)!=0)
	{
		bits |= MAT_SPECULAR_BIT;
		if (mat.texId_specular >=0)
		{
			specular *= texture(textureSamplers[mat.texId_specular], texCoord).xyz;
		}

		float sum_s = specular.x + specular.y + specular.z;
		float sum_d = diffuse.x + diffuse.y + diffuse.z;
		k = sum_s / (sum_s + sum_d);
	}

	vec3 emission = mat.emission;
	if ((mat.mask&4)!=0)
	{
		bits |= MAT_EMIT_BIT;
		if (mat.texId_emission >=0)
		{
			emission *= texture(textureSamplers[mat.texId_emission], texCoord).xyz;
		}
	}

	payload.t = gl_HitTNV;
	payload.normal = normal;
	payload.material_bits = bits;

	payload.color0 = emission;
	payload.color1 = diffuse;
	payload.color2 = specular;
	payload.f0 = k;
	payload.f1 = exp(1.0-mat.shininess);

}





