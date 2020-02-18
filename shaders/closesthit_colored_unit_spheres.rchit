#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable

#include "payload.shinc"
#include "bindings.h"

layout(location = 0) rayPayloadInNV Payload payload;
hitAttributeNV vec3 hitpoint;

struct ColoredUnitSphere
{
	mat3 normalMat;
	vec4 color;
	uint material; // 0: lamertian, 1: metal, 2: dielectric, 3: emissive
	float fuzz;
	float ref_idx;
};


layout(std430, binding = BINDING_ColoredUnitSphere) buffer Params
{
	ColoredUnitSphere[] coloredUnitSpheres;	
};

const uint cPresets[4] = {
	MAT_OPAQUE_BIT | MAT_DIFFUSE_BIT,
	MAT_OPAQUE_BIT | MAT_SPECULAR_BIT,
	MAT_ABSORB_BIT,
	MAT_OPAQUE_BIT | MAT_EMIT_BIT
};

void main()
{
	ColoredUnitSphere instance = coloredUnitSpheres[gl_InstanceCustomIndexNV];
	vec3 normal = normalize(instance.normalMat * hitpoint);

	payload.material_bits = cPresets[instance.material];
	payload.t = gl_HitTNV;
	payload.color0 = instance.color.xyz;
	payload.color1 = instance.color.xyz;
	payload.color2 = instance.color.xyz;
	payload.normal = normal;  	
  	payload.f1 = instance.fuzz;
	
	if (instance.material == 0) payload.f0 = 0.0;
	if (instance.material == 1) payload.f0 = 1.0;
	if (instance.material == 2) payload.f0 = instance.ref_idx;	  	

}


