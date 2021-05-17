#include "context.h"
#include "TexturedTriangleList.h"
#include "shaders/common/bindings.h"

void TexturedTriangleList::_blas_create()
{
	const Context& ctx = Context::get_context();

	VkAccelerationStructureGeometryKHR acceleration_geometry{};
	acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	acceleration_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	acceleration_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	acceleration_geometry.geometry.triangles.vertexData.deviceAddress = m_vertexBuffer->get_device_address();
	acceleration_geometry.geometry.triangles.maxVertex = (unsigned)(m_vertexBuffer->size() / sizeof(Vertex));
	acceleration_geometry.geometry.triangles.vertexStride = sizeof(Vertex);
	acceleration_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

	VkAccelerationStructureBuildGeometryInfoKHR geoBuildInfo{};
	geoBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	geoBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	geoBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	geoBuildInfo.geometryCount = 1;
	geoBuildInfo.pGeometries = &acceleration_geometry;

	VkAccelerationStructureBuildRangeInfoKHR range{};
	range.primitiveCount = (unsigned)(m_vertexBuffer->size() / sizeof(Vertex) / 3);
	range.primitiveOffset = 0;
	range.firstVertex = 0;
	range.transformOffset = 0;
	const VkAccelerationStructureBuildRangeInfoKHR* ranges = &range;

	m_blas = new BaseLevelAS(geoBuildInfo, &acceleration_geometry, &ranges);
}


TexturedTriangleList::TexturedTriangleList(const glm::mat4x4& model, const std::vector<Vertex>& vertices, const std::vector<Material>& materials, const int* materialIdx) : Geometry(model)
{
	m_vertexBuffer = new DeviceBuffer(sizeof(Vertex)* vertices.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	m_vertexBuffer->upload(vertices.data());
	m_materialBuffer = new DeviceBuffer(sizeof(Material)* materials.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	m_materialBuffer->upload(materials.data());
	m_materialIdxBuffer = new DeviceBuffer(sizeof(int)* vertices.size() /3, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
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
