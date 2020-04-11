#version 460
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec2 vUV;

void main() 
{
	// grid should be (0, 0), (2, 0), (0, 2)
	vec2 grid = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);

	// pos should be (-1, -1), (3, -1), (-1, 3)
	vec2 vpos = grid * vec2(2.0f, 2.0f) + vec2(-1.0f, -1.0f);

	gl_Position = vec4(vpos, 1.0f, 1.0f);

	vUV = grid;
}

