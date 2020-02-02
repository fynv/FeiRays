#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_NV_ray_tracing : enable

layout(location = 1) rayPayloadInNV bool isShadowed;

void main()
{
  isShadowed = false;
}
