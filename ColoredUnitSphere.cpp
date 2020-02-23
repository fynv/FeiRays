#include "context.h"
#include "ColoredUnitSphere.h"
#include "shaders/bindings.h"

ColoredUnitSphere::ColoredUnitSphere(const glm::mat4x4& model, const Material& material) : Geometry(model)
{
	m_material = material;

	static float s_aabb[6] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

	DeviceBuffer aabb_buf(sizeof(float) * 6, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	aabb_buf.upload(s_aabb);
	_blas_create_procedure(&aabb_buf);
}

ColoredUnitSphere::~ColoredUnitSphere()
{
	
}

struct SphereView
{
	glm::mat3x4 normalMat;
	glm::vec4 color;
	MaterialType type;
	float fuzz;
	float ref_idx;
	float density;
};

GeoCls ColoredUnitSphere::cls() const
{
	static const char s_name[] = "ColoredUnitSphere";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(SphereView);
	cls.binding_view = BINDING_ColoredUnitSphere;
	cls.fn_intersection = "intersection_unit_spheres";
	cls.fn_closesthit = "closesthit_colored_unit_spheres";
	return cls;
}

void ColoredUnitSphere::get_view(void* view_buf) const
{
	SphereView& view = *(SphereView*)view_buf;
	view.normalMat = m_norm_mat;
	view.color = { m_material.color, 1.0f };
	view.type = m_material.type;
	view.fuzz = m_material.fuzz;
	view.ref_idx = m_material.ref_idx;
	view.density = m_material.density;
}

