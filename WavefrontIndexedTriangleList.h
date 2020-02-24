#pragma once

#include "PathTracer.h"

class WavefrontIndexedTriangleList : public Geometry
{
public:
	struct Material
	{
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 emission;
		float shininess;
		int texId_diffuse;
		int texId_specular;
		int texId_emission;
		int texId_bumpmap;
		int mask; // bit 0 : has diffuse, bit 1: has specular, bit 2: has emission
	};

	struct Index
	{
		int vertex_index;
		int normal_index;
		int texcoord_index;
	};

	WavefrontIndexedTriangleList(const glm::mat4x4& model,
		const std::vector<glm::vec3>& positions, 
		const std::vector<glm::vec3>& normals,
		const std::vector<glm::vec2>& texcoords,
		const std::vector<Index>& indices,
		std::vector<Material>& materials, const int* materialIdx);

	virtual ~WavefrontIndexedTriangleList();

	virtual GeoCls cls() const;
	virtual void get_view(void* view_buf) const;

private:
	DeviceBuffer* m_normalBuffer;
	DeviceBuffer* m_texcoordBuffer;
	DeviceBuffer* m_indexBuffer;
	DeviceBuffer* m_materialBuffer;
	DeviceBuffer* m_faceBuffer;
};
