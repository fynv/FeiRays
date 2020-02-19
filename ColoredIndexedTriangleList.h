#pragma once

#include "PathTracer.h"
#include "Material.h"

class ColoredIndexedTriangleList : public Geometry
{
public:
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
	};

	DeviceBuffer* vertex_buffer() const { return m_vertexBuffer; }
	DeviceBuffer* index_buffer() const { return m_indexBuffer; }

	ColoredIndexedTriangleList(const glm::mat4x4& model, const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, const Material& material);

	virtual ~ColoredIndexedTriangleList();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;

private:
	void _blas_create();

	unsigned m_vertexCount;
	unsigned m_indexCount;

	DeviceBuffer* m_vertexBuffer;
	DeviceBuffer* m_indexBuffer;

	Material m_material;

};
