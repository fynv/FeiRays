#pragma once

#include "PathTracer.h"
#include "Material.h"

class ColoredUnitSphere : public Geometry
{
public:
	ColoredUnitSphere(const glm::mat4x4& model, const Material& material);
	virtual ~ColoredUnitSphere();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;


private:
	Material m_material;

};
