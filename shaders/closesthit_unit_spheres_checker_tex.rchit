#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable

#include "payload.shinc"
#include "bindings.h"

layout(location = 0) rayPayloadInNV Payload payload;
hitAttributeNV vec3 hitpoint;

struct UnitSphereCheckerTex
{
	mat4 modelMat;
	mat3 normalMat;
	vec4 color1;
	vec4 color2;
	float interval;
};

layout(std430, binding = BINDING_UnitSphereCheckerTex) buffer Params
{
	UnitSphereCheckerTex[] UnitSpheres_checker_tex;	
};


void main()
{
	UnitSphereCheckerTex instance = UnitSpheres_checker_tex[gl_InstanceCustomIndexNV];

	vec4 pos = instance.modelMat*vec4(hitpoint, 1.0);
	int i_x = int(pos.x / instance.interval);
	int i_y = int(pos.y / instance.interval);
	int i_z = int(pos.z / instance.interval);
	vec4 color = (i_x+i_y+i_z)%2 == 0 ? instance.color1 : instance.color2;

	vec3 normal = normalize(instance.normalMat * hitpoint);

	payload.t = gl_HitTNV;
	payload.material_bits = MAT_OPAQUE_BIT | MAT_DIFFUSE_BIT;
	payload.color1 = color.xyz;
	payload.f0 = 0.0;
	payload.normal = normal;

}


