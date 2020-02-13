#include "context.h"
#include "UnitSphereCheckerTex.h"

void UnitSphereCheckerTex::_blas_create()
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


UnitSphereCheckerTex::UnitSphereCheckerTex(const glm::mat4x4& model, float interval, glm::vec3 color1, glm::vec3 color2) : Geometry(model)
{
	m_color1 = color1;
	m_color2 = color2;
	m_interval = interval;

	static float s_aabb[6] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

	m_aabb_buf = new DeviceBuffer(sizeof(float) * 6, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_aabb_buf->upload(s_aabb);

	_blas_create();
}

UnitSphereCheckerTex::~UnitSphereCheckerTex()
{
	delete m_aabb_buf;
}

struct SphereView
{
	glm::mat4x4 modelMat;
	glm::mat3x4 normalMat;
	glm::vec4 color1;
	glm::vec4 color2;
	float interval;
};


GeoCls UnitSphereCheckerTex::cls() const
{
	static const char s_name[] = "UnitSphereCheckerTex";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(SphereView);
	cls.binding_view = 6;
	cls.fn_intersection = "../shaders/intersection_unit_spheres.spv";
	cls.fn_closesthit = "../shaders/closesthit_unit_spheres_checker_tex.spv";
	return cls;
}

void UnitSphereCheckerTex::get_view(void* view_buf) const
{
	SphereView& view = *(SphereView*)view_buf;
	view.modelMat = m_model;
	view.normalMat = m_norm_mat;
	view.color1 = { m_color1, 1.0f };
	view.color2 = { m_color2, 1.0f };
	view.interval = m_interval;
}
