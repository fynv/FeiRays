#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#include "../common/payload.shinc"
#include "../common/bindings.h"

layout(location = 0) rayPayloadInEXT Payload payload;
hitAttributeEXT vec2 attribs;

layout(binding = 3) uniform sampler2D[] textureSamplers;

struct Index
{
	int normal_index;
	int texcoord_index;
};

struct Material
{
	vec3 diffuse;
	vec3 specular;
	vec3 emission;
	float shininess;
	int texId_diffuse;
	int texId_specular;
	int texId_emission;
	int texId_bumpmap;
	int texId_mask;
	int mask;
};

struct Face
{
	vec3 T;
	vec3 B;
	int materialIdx;
};

layout(buffer_reference, scalar, buffer_reference_align = 4) buffer Vec3Buf
{
	vec3 v;
};

layout(buffer_reference, scalar, buffer_reference_align = 4) buffer Vec2Buf
{
	vec2 v;
};

layout(buffer_reference, scalar, buffer_reference_align = 4) buffer IndexBuf
{
	Index i;
};

layout(buffer_reference, scalar, buffer_reference_align = 4) buffer MaterialBuf
{
	Material m;
};

layout(buffer_reference, scalar, buffer_reference_align = 4) buffer FaceBuf
{
	Face f;
};

struct WavefrontIndexedTriangleList
{
	mat3 normalMat;
	Vec3Buf normalBuf;
	Vec2Buf texcoordBuf;
	IndexBuf indexBuf;
	MaterialBuf materialBuf;
	FaceBuf faceBuf;
};

layout(std430, binding = BINDING_WavefrontIndexedTriangleList) buffer Params
{
	WavefrontIndexedTriangleList[] wavefrontIndexedTriangleList;
};


void main()
{
	payload.t = gl_HitTEXT;

	WavefrontIndexedTriangleList instance = wavefrontIndexedTriangleList[gl_InstanceCustomIndexEXT];

	Index i0 = instance.indexBuf[3 * gl_PrimitiveID].i;
	Index i1 = instance.indexBuf[3 * gl_PrimitiveID + 1].i;
	Index i2 = instance.indexBuf[3 * gl_PrimitiveID + 2].i;

	Face face = instance.faceBuf[gl_PrimitiveID].f;
	Material mat = instance.materialBuf[face.materialIdx].m;

	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec2 texCoord;

	if (i0.texcoord_index>=0)
	{
		vec2 texCoord0 = instance.texcoordBuf[i0.texcoord_index].v;
		vec2 texCoord1 = instance.texcoordBuf[i1.texcoord_index].v;
		vec2 texCoord2 = instance.texcoordBuf[i2.texcoord_index].v;
		texCoord = texCoord0 * barycentrics.x + texCoord1 * barycentrics.y + texCoord2 * barycentrics.z;
	}

	if (mat.texId_mask >= 0)
	{
		float mask = texture(textureSamplers[mat.texId_mask], texCoord).x;
		if (mask<0.5)
		{
			payload.material_bits = 0;
			return;
		}
	}

	vec3 normal;
	{
		vec3 norm0 = instance.normalBuf[i0.normal_index].v;
		vec3 norm1 = instance.normalBuf[i1.normal_index].v;
		vec3 norm2 = instance.normalBuf[i2.normal_index].v;
		normal = norm0 * barycentrics.x + norm1 * barycentrics.y + norm2 * barycentrics.z;

		if (mat.texId_bumpmap >= 0)
		{
			normal = normalize(normal);
			vec3 bump = texture(textureSamplers[mat.texId_bumpmap], texCoord).xyz;
			bump = 2.0 * bump - 1.0;
			vec3 new_normal = bump.x*face.T + bump.y*face.B + bump.z*normal;
			if (dot(new_normal, normal)>0.0) normal = new_normal;
		}

		normal = normalize(instance.normalMat * normal);
	}

	payload.normal = normal;

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

	payload.material_bits = bits;

	payload.color0 = emission;
	payload.color1 = diffuse;
	payload.color2 = specular;
	payload.f0 = k;
	payload.f1 = 1.0 / mat.shininess;

}





