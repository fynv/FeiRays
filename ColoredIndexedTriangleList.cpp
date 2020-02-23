#include "context.h"
#include "ColoredIndexedTriangleList.h"
#include "shaders/bindings.h"

ColoredIndexedTriangleList::ColoredIndexedTriangleList(const glm::mat4x4& model, const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, const Material& material) : Geometry(model)
{
	m_material = material; 

	std::vector<glm::vec3> positions(vertices.size());
	std::vector<glm::vec3> norms(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		positions[i] = vertices[i].Position;
		norms[i] = vertices[i].Normal;
	}

	m_vertexBuffer = new DeviceBuffer(sizeof(glm::vec3)* vertices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_vertexBuffer->upload(norms.data());
	m_indexBuffer = new DeviceBuffer(sizeof(unsigned)*indices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_indexBuffer->upload(indices.data());

	DeviceBuffer posBuf(sizeof(glm::vec3)* vertices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	posBuf.upload(positions.data());

	_blas_create_indexed_triangles(&posBuf, m_indexBuffer);
}

ColoredIndexedTriangleList::~ColoredIndexedTriangleList()
{
	delete m_indexBuffer;
	delete m_vertexBuffer;
}


struct TriangleMeshView
{
	glm::mat3x4 normalMat;
	glm::vec4 color;
	VkDeviceAddress vertexBuf;
	VkDeviceAddress indexBuf;
	MaterialType type;
	float fuzz;
	float ref_idx;
	float density;
};

GeoCls ColoredIndexedTriangleList::cls() const
{
	static const char s_name[] = "ColoredIndexedTriangleList";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(TriangleMeshView);
	cls.binding_view = BINDING_ColoredIndexedTriangleList;
	cls.fn_intersection = nullptr;
	cls.fn_closesthit = "closesthit_colored_indexed_triangle_lists";
	return cls;
}

void ColoredIndexedTriangleList::get_view(void* view_buf) const
{
	TriangleMeshView& view = *(TriangleMeshView*)view_buf;
	view.normalMat = m_norm_mat;
	view.color = { m_material.color, 1.0f };
	view.vertexBuf = m_vertexBuffer->get_device_address();
	view.indexBuf = m_indexBuffer->get_device_address();
	view.type = m_material.type;
	view.fuzz = m_material.fuzz;
	view.ref_idx = m_material.ref_idx;
	view.density = m_material.density;
}