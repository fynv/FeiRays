#pragma once

#include "PathTracer.h"

class TexturedTriangleList : public Geometry
{
public:
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
	};

	struct Material
	{
		glm::vec3 diffuse;
		int textureId;
	};

	DeviceBuffer* vertex_buffer() const { return m_vertexBuffer; }
	DeviceBuffer* material_buffer() const { return m_materialBuffer; }
	DeviceBuffer* material_index_buffer() const { return m_materialIdxBuffer; }

	TexturedTriangleList(const glm::mat4x4& model, const std::vector<Vertex>& vertices, const std::vector<Material>& materials, const int* materialIdx);
	virtual ~TexturedTriangleList();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;

private:
	void _blas_create();

	unsigned m_vertexCount;
	unsigned m_materialCount;

	DeviceBuffer* m_vertexBuffer;
	DeviceBuffer* m_materialBuffer;
	DeviceBuffer* m_materialIdxBuffer;
};

