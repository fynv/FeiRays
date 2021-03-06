#include "context.h"
#include "UnitSphereCheckerTex.h"
#include "shaders/common/bindings.h"

UnitSphereCheckerTex::UnitSphereCheckerTex(const glm::mat4x4& model, float interval, const glm::vec3& color1, const glm::vec3& color2) : Geometry(model)
{
	m_color1 = color1;
	m_color2 = color2;
	m_interval = interval;

	static float s_aabb[6] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
	DeviceBuffer aabb_buf(sizeof(float) * 6, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	aabb_buf.upload(s_aabb);
	_blas_create_procedure(&aabb_buf);
}

UnitSphereCheckerTex::~UnitSphereCheckerTex()
{

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
	static const char s_fn_rint[] = "geometry/intersection_unit_spheres.spv";
	static const char s_fn_rchit[] = "geometry/closesthit_unit_spheres_checker_tex.spv";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(SphereView);
	cls.binding_view = BINDING_UnitSphereCheckerTex;
	cls.fn_intersection = s_fn_rint;
	cls.fn_closesthit = s_fn_rchit;
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
