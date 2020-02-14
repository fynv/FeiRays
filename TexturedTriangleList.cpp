#include "context.h"
#include "TexturedTriangleList.h"
#include "shaders/bindings.h"

void TexturedTriangleList::_blas_create()
{
	const Context& ctx = Context::get_context();

	VkGeometryNV geometry = {};
	geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	geometry.geometry.triangles = {};
	geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	geometry.geometry.triangles.vertexData = m_vertexBuffer->buf();
	geometry.geometry.triangles.vertexOffset = 0;
	geometry.geometry.triangles.vertexCount = (unsigned)(m_vertexBuffer->size() / sizeof(Vertex));
	geometry.geometry.triangles.vertexStride = sizeof(Vertex);
	geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	geometry.geometry.triangles.indexData = nullptr;
	geometry.geometry.triangles.indexOffset = 0;
	geometry.geometry.triangles.indexCount = 0;
	geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
	geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
	geometry.geometry.triangles.transformOffset = 0;
	geometry.geometry.aabbs = { VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV };
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;

	m_blas = new BaseLevelAS(1, &geometry);
}


TexturedTriangleList::TexturedTriangleList(const glm::mat4x4& model, const std::vector<Vertex>& vertices, const std::vector<Material>& materials, const int* materialIdx) : Geometry(model)
{
	m_vertexCount = (unsigned)vertices.size();
	m_materialCount = (unsigned)materials.size();

	m_vertexBuffer = new DeviceBuffer(sizeof(Vertex)* m_vertexCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_vertexBuffer->upload(vertices.data());
	m_materialBuffer = new DeviceBuffer(sizeof(Material)* m_materialCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_materialBuffer->upload(materials.data());
	m_materialIdxBuffer = new DeviceBuffer(sizeof(int)* m_vertexCount/3, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
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
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(TriangleMeshView);
	cls.binding_view = BINDING_TexturedTriangleList;
	cls.fn_intersection = nullptr;
	cls.fn_closesthit = "../shaders/closesthit_textured_triangle_lists.spv";
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
