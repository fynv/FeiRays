#pragma once

#include "PathTracer.h"

class UnitSphereCheckerTex : public Geometry
{
public:
	UnitSphereCheckerTex(const glm::mat4x4& model, float interval, const glm::vec3& color1 = { 0.0f, 0.0f, 0.0f }, const glm::vec3& color2 = { 1.0f, 1.0f, 1.0f });
	virtual ~UnitSphereCheckerTex();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;


private:
	glm::vec3 m_color1;
	glm::vec3 m_color2;

	float m_interval;


};
