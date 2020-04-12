#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "../common/payload.shinc"
#include "../common/bindings.h"

layout(location = 0) rayPayloadInEXT Payload payload;
hitAttributeEXT vec3 hitpoint;

layout(binding = 3) uniform sampler2D[] textureSamplers;

struct TexturedUnitSphere
{
	mat3 normalMat;
	vec4 color;
	int textureId;
};

layout(std430, binding = BINDING_TexturedUnitSphere) buffer Params
{
	TexturedUnitSphere[] texturedUnitSpheres;	
};


void main()
{
	TexturedUnitSphere instance = texturedUnitSpheres[gl_InstanceCustomIndexEXT];
	vec3 normal = normalize(instance.normalMat * hitpoint);
	
	vec3 c = instance.color.xyz;
	vec2 texCoord;
	texCoord.y = asin(hitpoint.y)/radians(180.0) + 0.5;
	texCoord.x = atan(hitpoint.x, hitpoint.z)/radians(360.0) + 0.5;
	c*= texture(textureSamplers[instance.textureId], texCoord).xyz;

	payload.t = gl_HitTEXT;
	payload.material_bits = MAT_OPAQUE_BIT | MAT_DIFFUSE_BIT;
	payload.color1 = c;
	payload.f0 = 0.0;
	payload.normal = normal;

}


