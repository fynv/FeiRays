#pragma once

#include "PathTracer.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
};

class ColoredTriangleList : public Geometry
{
public:
	DeviceBuffer* vertex_buffer() const { return m_vertexBuffer; }
	DeviceBuffer* index_buffer() const { return m_indexBuffer; }

	ColoredTriangleList(const glm::mat4x4& model, const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, glm::vec3 color = { 1.0f, 1.0f, 1.0f });
	virtual ~ColoredTriangleList();

	virtual GeoCls cls() const;
	virtual void get_view(char* view_buf) const;

private:
	void _blas_create();

	unsigned m_vertexCount;
	unsigned m_indexCount;

	DeviceBuffer* m_vertexBuffer;
	DeviceBuffer* m_indexBuffer;

	glm::vec3 m_color;

};
