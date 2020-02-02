#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable

#include "payload.shinc"

layout(location = 0) rayPayloadInNV Payload payload;
hitAttributeNV vec4 hitpoint;

struct Sphere
{
	mat3 normalMat;
	vec4 color;
};


layout(std430, binding = 4) buffer Params
{
	Sphere[] spheres;	
};

void main()
{
	Sphere instance = spheres[gl_InstanceCustomIndexNV];
	vec3 normal = normalize(instance.normalMat * hitpoint.xyz) * hitpoint.w;
	payload.color_dis = vec4(instance.color.xyz, gl_HitTNV);
  	payload.normal = vec4(normal, 0.0);
}

