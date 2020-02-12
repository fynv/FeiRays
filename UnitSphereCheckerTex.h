#pragma once

#include "PathTracer.h"

class UnitSphereCheckerTex : public Geometry
{
public:
	DeviceBuffer* aabb_buffer() const { return m_aabb_buf; }

	UnitSphereCheckerTex(const glm::mat4x4& model, float interval, glm::vec3 color1 = { 0.0f, 0.0f, 0.0f }, glm::vec3 color2 = { 1.0f, 1.0f, 1.0f });
	virtual ~UnitSphereCheckerTex();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;


private:
	void _blas_create();
	DeviceBuffer* m_aabb_buf;

	glm::vec3 m_color1;
	glm::vec3 m_color2;

	float m_interval;


};
