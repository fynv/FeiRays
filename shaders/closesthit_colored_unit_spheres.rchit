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
	uint material; // 0: lamertian, 1: metal, 2: dielectric	
	float fuzz;
	float ref_idx;
};


layout(std430, binding = BINDING_ColoredUnitSphere) buffer Params
{
	ColoredUnitSphere[] coloredUnitSpheres;	
};

void main()
{
	ColoredUnitSphere instance = coloredUnitSpheres[gl_InstanceCustomIndexNV];
	vec3 normal = normalize(instance.normalMat * hitpoint);
	payload.color_dis = vec4(instance.color.xyz, gl_HitTNV);
  	payload.normal = vec4(normal, 0.0);
  	payload.material = instance.material;
  	payload.fuzz = instance.fuzz;
  	payload.ref_idx = instance.ref_idx;
}


