#include "context.h"
#include "TexturedUnitSphere.h"
#include "shaders/bindings.h"

void TexturedUnitSphere::_blas_create()
{
	const Context& ctx = Context::get_context();

	VkGeometryNV geometry = {};
	geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
	geometry.geometry.triangles = {};
	geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	geometry.geometry.aabbs = {};
	geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	geometry.geometry.aabbs.aabbData = m_aabb_buf->buf();
	geometry.geometry.aabbs.offset = 0;
	geometry.geometry.aabbs.numAABBs = 1;
	geometry.geometry.aabbs.stride = 0;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;

	m_blas = new BaseLevelAS(1, &geometry);
}

TexturedUnitSphere::TexturedUnitSphere(const glm::mat4x4& model, int tex_id, const glm::vec3& color) : Geometry(model)
{
	m_color = color;
	m_tex_id = tex_id;

	static float s_aabb[6] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

	m_aabb_buf = new DeviceBuffer(sizeof(float) * 6, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_aabb_buf->upload(s_aabb);

	_blas_create();
}

TexturedUnitSphere::~TexturedUnitSphere()
{
	delete m_aabb_buf;
}


struct SphereView
{
	glm::mat3x4 normalMat;
	glm::vec4 color;
	int tex_id;
};

GeoCls TexturedUnitSphere::cls() const
{
	static const char s_name[] = "TexturedUnitSphere";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(SphereView);
	cls.binding_view = BINDING_TexturedUnitSphere;
	cls.fn_intersection = "../shaders/intersection_unit_spheres.spv";
	cls.fn_closesthit = "../shaders/closesthit_textured_unit_spheres.spv";
	return cls;
}

void TexturedUnitSphere::get_view(void* view_buf) const
{
	SphereView& view = *(SphereView*)view_buf;
	view.normalMat = m_norm_mat;
	view.color = { m_color, 1.0f };
	view.tex_id = m_tex_id;
}

