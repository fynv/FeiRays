#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_tracing : enable

#include "../common/payload.shinc"
#include "../common/bindings.h"

layout(location = 0) rayPayloadInEXT Payload payload;
hitAttributeEXT vec3 hitpoint;


struct SphereLight
{
	vec4 center_radius;
	vec4 color;
};


layout(std430, binding = BINDING_SphereLight) buffer SpheresLights
{
	SphereLight[] sphereLights;	
};


void main()
{
	SphereLight instance = sphereLights[gl_InstanceCustomIndexEXT];
	vec3 normal = normalize(hitpoint);

	payload.material_bits = MAT_OPAQUE_BIT | MAT_EMIT_BIT | MAT_LIGHT_SOURCE_BIT;
	payload.f0 = intBitsToFloat(gl_InstanceCustomIndexEXT);
	payload.t = gl_HitTEXT;
	payload.color0 = instance.color.xyz;
	payload.normal = normal;  	
}

