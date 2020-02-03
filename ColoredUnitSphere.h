#pragma once

#include "PathTracer.h"

class ColoredUnitSphere : public Geometry
{
public:
	DeviceBuffer* aabb_buffer() const { return m_aabb_buf; }

	ColoredUnitSphere(const glm::mat4x4& model, glm::vec3 color = { 1.0f, 1.0f, 1.0f });
	virtual ~ColoredUnitSphere();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;


private:
	void _blas_create();
	DeviceBuffer* m_aabb_buf;

	glm::vec3 m_color;

};
