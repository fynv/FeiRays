#pragma once

#include "PathTracer.h"
#include "Material.h"

class ColoredUnitSphere : public Geometry
{
public:
	DeviceBuffer* aabb_buffer() const { return m_aabb_buf; }

	ColoredUnitSphere(const glm::mat4x4& model, const Material& material);
	virtual ~ColoredUnitSphere();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;


private:
	void _blas_create();
	DeviceBuffer* m_aabb_buf;
	Material m_material;

};
