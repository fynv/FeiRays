#include "context.h"
#include "WavefrontIndexedTriangleList.h"
#include "shaders/bindings.h"

WavefrontIndexedTriangleList::WavefrontIndexedTriangleList(const glm::mat4x4& model,
	const std::vector<glm::vec3>& positions,
	const std::vector<glm::vec3>& normals,
	const std::vector<glm::vec2>& texcoords,
	const std::vector<Index>& indices,
	std::vector<Material>& materials, const int* materialIdx) : Geometry(model)
{
	m_normalBuffer = new DeviceBuffer(sizeof(glm::vec3)* normals.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_normalBuffer->upload(normals.data());
	m_texcoordBuffer = new DeviceBuffer(sizeof(glm::vec2)* texcoords.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_texcoordBuffer->upload(texcoords.data());
	m_indexBuffer = new DeviceBuffer(sizeof(Index)* indices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_indexBuffer->upload(indices.data());
	m_materialBuffer = new DeviceBuffer(sizeof(Material)* materials.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_materialBuffer->upload(materials.data());
	m_materialIdxBuffer = new DeviceBuffer(sizeof(int)* indices.size() / 3, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_materialIdxBuffer->upload(materialIdx);

	std::vector<unsigned> pos_indices(indices.size());
	for (size_t i = 0; i < indices.size(); i++)
		pos_indices[i] = indices[i].vertex_index;

	DeviceBuffer positionBuffer(sizeof(glm::vec3)* positions.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	positionBuffer.upload(positions.data());

	DeviceBuffer pos_index_buffer(sizeof(unsigned)* indices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	pos_index_buffer.upload(pos_indices.data());

	_blas_create_indexed_triangles(&positionBuffer, &pos_index_buffer);
}

WavefrontIndexedTriangleList::~WavefrontIndexedTriangleList()
{
	delete m_materialIdxBuffer;
	delete m_materialBuffer;
	delete m_indexBuffer;
	delete m_texcoordBuffer;
	delete m_normalBuffer;
}


struct TriangleMeshView
{
	glm::mat3x4 normalMat;
	VkDeviceAddress normalBuf;
	VkDeviceAddress texcoordBuf;
	VkDeviceAddress indexBuf;
	VkDeviceAddress materialBuf;
	VkDeviceAddress materialIdxBuf;
};


GeoCls WavefrontIndexedTriangleList::cls() const
{
	static const char s_name[] = "WavefrontIndexedTriangleList";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(TriangleMeshView);
	cls.binding_view = BINDING_WavefrontIndexedTriangleList;
	cls.fn_intersection = nullptr;
	cls.fn_closesthit = "closesthit_wavefront_indexed_triangle_lists";
	return cls;
}

void WavefrontIndexedTriangleList::get_view(void* view_buf) const
{
	TriangleMeshView& view = *(TriangleMeshView*)view_buf;
	view.normalMat = m_norm_mat;
	view.normalBuf = m_normalBuffer->get_device_address();
	view.texcoordBuf = m_texcoordBuffer->get_device_address();
	view.indexBuf = m_indexBuffer->get_device_address();
	view.materialBuf = m_materialBuffer->get_device_address();
	view.materialIdxBuf = m_materialIdxBuffer->get_device_address();
}
