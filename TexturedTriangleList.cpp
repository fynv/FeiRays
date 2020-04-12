#include "context.h"
#include "TexturedTriangleList.h"
#include "shaders/common/bindings.h"

void TexturedTriangleList::_blas_create()
{
	const Context& ctx = Context::get_context();

	VkAccelerationStructureCreateGeometryTypeInfoKHR acceleration_create_geometry_info{};
	acceleration_create_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
	acceleration_create_geometry_info.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	acceleration_create_geometry_info.maxPrimitiveCount = (unsigned)(m_vertexBuffer->size() / sizeof(Vertex)/3);
	acceleration_create_geometry_info.indexType = VK_INDEX_TYPE_NONE_KHR;
	acceleration_create_geometry_info.maxVertexCount = (unsigned)(m_vertexBuffer->size() / sizeof(Vertex));
	acceleration_create_geometry_info.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	acceleration_create_geometry_info.allowsTransforms = VK_FALSE;

	VkAccelerationStructureGeometryKHR acceleration_geometry{};
	acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	acceleration_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	acceleration_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	acceleration_geometry.geometry.triangles.vertexData.deviceAddress = m_vertexBuffer->get_device_address();
	acceleration_geometry.geometry.triangles.vertexStride = sizeof(Vertex);
	acceleration_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

	VkAccelerationStructureBuildOffsetInfoKHR acceleration_build_offset_info{};
	acceleration_build_offset_info.primitiveCount = (unsigned)(m_vertexBuffer->size() / sizeof(Vertex) / 3);
	acceleration_build_offset_info.primitiveOffset = 0x0;
	acceleration_build_offset_info.firstVertex = 0;
	acceleration_build_offset_info.transformOffset = 0x0;

	m_blas = new BaseLevelAS(1, &acceleration_create_geometry_info, &acceleration_geometry, &acceleration_build_offset_info);
}


TexturedTriangleList::TexturedTriangleList(const glm::mat4x4& model, const std::vector<Vertex>& vertices, const std::vector<Material>& materials, const int* materialIdx) : Geometry(model)
{
	m_vertexBuffer = new DeviceBuffer(sizeof(Vertex)* vertices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
	m_vertexBuffer->upload(vertices.data());
	m_materialBuffer = new DeviceBuffer(sizeof(Material)* materials.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
	m_materialBuffer->upload(materials.data());
	m_materialIdxBuffer = new DeviceBuffer(sizeof(int)* vertices.size() /3, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
	m_materialIdxBuffer->upload(materialIdx);

	_blas_create();
}

TexturedTriangleList::~TexturedTriangleList()
{
	delete m_materialIdxBuffer;
	delete m_materialBuffer;
	delete m_vertexBuffer;
}

struct TriangleMeshView
{
	glm::mat3x4 normalMat;
	VkDeviceAddress vertexBuf;
	VkDeviceAddress materialBuf;
	VkDeviceAddress materialIdxBuf;
};

GeoCls TexturedTriangleList::cls() const
{
	static const char s_name[] = "TexturedTriangleList";
	static const char s_fn_rchit[] = "geometry/closesthit_textured_triangle_lists.spv";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(TriangleMeshView);
	cls.binding_view = BINDING_TexturedTriangleList;
	cls.fn_intersection = nullptr;
	cls.fn_closesthit = s_fn_rchit;
	return cls;
}

void TexturedTriangleList::get_view(void* view_buf) const
{
	TriangleMeshView& view = *(TriangleMeshView*)view_buf;
	view.normalMat = m_norm_mat;
	view.vertexBuf = m_vertexBuffer->get_device_address();
	view.materialBuf = m_materialBuffer->get_device_address();
	view.materialIdxBuf = m_materialIdxBuffer->get_device_address();	
}
