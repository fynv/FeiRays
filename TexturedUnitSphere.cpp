#include "context.h"
#include "TexturedUnitSphere.h"
#include "shaders/common/bindings.h"

TexturedUnitSphere::TexturedUnitSphere(const glm::mat4x4& model, int tex_id, const glm::vec3& color) : Geometry(model)
{
	m_color = color;
	m_tex_id = tex_id;

	static float s_aabb[6] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

	DeviceBuffer aabb_buf(sizeof(float) * 6, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	aabb_buf.upload(s_aabb);
	_blas_create_procedure(&aabb_buf);
}

TexturedUnitSphere::~TexturedUnitSphere()
{

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
	static const char s_fn_rint[] = "geometry/intersection_unit_spheres.spv";
	static const char s_fn_rchit[] = "geometry/closesthit_textured_unit_spheres.spv";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(SphereView);
	cls.binding_view = BINDING_TexturedUnitSphere;
	cls.fn_intersection = s_fn_rint;
	cls.fn_closesthit = s_fn_rchit;
	return cls;
}

void TexturedUnitSphere::get_view(void* view_buf) const
{
	SphereView& view = *(SphereView*)view_buf;
	view.normalMat = m_norm_mat;
	view.color = { m_color, 1.0f };
	view.tex_id = m_tex_id;
}


