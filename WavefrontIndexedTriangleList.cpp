#include "context.h"
#include "WavefrontIndexedTriangleList.h"
#include "shaders/common/bindings.h"

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

	struct Index2
	{
		int normal_index;
		int texcoord_index;
	};

	std::vector<Index2> indices2(indices.size());
	for (size_t i = 0; i < indices.size(); i++)
	{
		indices2[i].normal_index = indices[i].normal_index;
		indices2[i].texcoord_index = indices[i].texcoord_index;
	}

	m_indexBuffer = new DeviceBuffer(sizeof(Index2)* indices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_indexBuffer->upload(indices2.data());
	m_materialBuffer = new DeviceBuffer(sizeof(Material)* materials.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_materialBuffer->upload(materials.data());

	struct Face
	{
		glm::vec3 T;
		glm::vec3 B;
		int materialIdx;
	};

	std::vector<Face> faces(indices.size() / 3);

	for (size_t i = 0; i < indices.size() / 3; i++)
	{
		faces[i].materialIdx = materialIdx[i];

		Index i0 = indices[i * 3];
		Index i1 = indices[i * 3 + 1];
		Index i2 = indices[i * 3 + 2];

		if (i0.texcoord_index >= 0)
		{
			glm::vec3 v0 = positions[i0.vertex_index];
			glm::vec3 v1 = positions[i1.vertex_index];
			glm::vec3 v2 = positions[i2.vertex_index];
			glm::vec2 texCoord0 = texcoords[i0.texcoord_index];
			glm::vec2 texCoord1 = texcoords[i1.texcoord_index];
			glm::vec2 texCoord2 = texcoords[i2.texcoord_index];

			glm::vec3 edge1 = v1 - v0;
			glm::vec3 edge2 = v2 - v0;
			glm::vec2 delta1 = texCoord1 - texCoord0;
			glm::vec2 delta2 = texCoord2 - texCoord0;

			float f = 1.0f / (delta1[0] * delta2[1] - delta2[0] * delta1[1]);
			faces[i].T = glm::normalize((f*delta2[1]) * edge1 - (f*delta1[1])*edge2);
			faces[i].B = glm::normalize((-f * delta2[0]) * edge1 + (f*delta1[0])*edge2);
		}
	}

	m_faceBuffer = new DeviceBuffer(sizeof(Face)* indices.size() / 3, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_faceBuffer->upload(faces.data());

	std::vector<unsigned> pos_indices(indices.size());
	for (size_t i = 0; i < indices.size(); i++)
		pos_indices[i] = indices[i].vertex_index;

	DeviceBuffer posBuf(sizeof(glm::vec3)* positions.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	posBuf.upload(positions.data());

	DeviceBuffer pos_index_buffer(sizeof(unsigned)* indices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	pos_index_buffer.upload(pos_indices.data());

	_blas_create_indexed_triangles(&posBuf, &pos_index_buffer);
}

WavefrontIndexedTriangleList::~WavefrontIndexedTriangleList()
{
	delete m_faceBuffer;
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
	VkDeviceAddress faceBuf;
};


GeoCls WavefrontIndexedTriangleList::cls() const
{
	static const char s_name[] = "WavefrontIndexedTriangleList";
	static const char s_fn_rchit[] = "geometry/closesthit_wavefront_indexed_triangle_lists.spv";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(TriangleMeshView);
	cls.binding_view = BINDING_WavefrontIndexedTriangleList;
	cls.fn_intersection = nullptr;
	cls.fn_closesthit = s_fn_rchit;
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
	view.faceBuf = m_faceBuffer->get_device_address();
}
