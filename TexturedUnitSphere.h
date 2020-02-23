#pragma once

#include "PathTracer.h"

class TexturedUnitSphere : public Geometry
{
public:
	TexturedUnitSphere(const glm::mat4x4& model, int tex_id, const glm::vec3& color = { 1.0f, 1.0f, 1.0f });
	virtual ~TexturedUnitSphere();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;

private:
	glm::vec3 m_color;
	int m_tex_id;
};