#include "context.h"
#include "ColoredIndexedTriangleList.h"
#include "shaders/bindings.h"

void ColoredIndexedTriangleList::_blas_create()
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
	geometry.geometry.triangles.indexData = m_indexBuffer->buf();
	geometry.geometry.triangles.indexOffset = 0;
	geometry.geometry.triangles.indexCount = (unsigned)(m_indexBuffer->size() / sizeof(unsigned));
	geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
	geometry.geometry.triangles.transformOffset = 0;
	geometry.geometry.aabbs = { VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV };
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;

	m_blas = new BaseLevelAS(1, &geometry);
}


ColoredIndexedTriangleList::ColoredIndexedTriangleList(const glm::mat4x4& model, const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, 
	glm::vec3 color, Material material, float fuzz, float ref_idx) : Geometry(model)
{
	m_color = color;
	m_material = material;
	m_fuzz = fuzz;
	m_ref_idx = ref_idx;

	m_vertexCount = (unsigned)vertices.size();
	m_indexCount = (unsigned)indices.size();

	m_vertexBuffer = new DeviceBuffer(sizeof(Vertex)* m_vertexCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_vertexBuffer->upload(vertices.data());
	m_indexBuffer = new DeviceBuffer(sizeof(unsigned)*m_indexCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT | VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_indexBuffer->upload(indices.data());

	_blas_create();
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
	unsigned material; // 0: lambertian, 1: metal, 2: dielectric	
	float fuzz;
	float ref_idx;
	int dummy;
};

GeoCls ColoredIndexedTriangleList::cls() const
{
	static const char s_name[] = "ColoredIndexedTriangleList";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(TriangleMeshView);
	cls.binding_view = BINDING_ColoredIndexedTriangleList;
	cls.fn_intersection = nullptr;
	cls.fn_closesthit = "../shaders/closesthit_colored_indexed_triangle_lists.spv";
	return cls;
}

void ColoredIndexedTriangleList::get_view(void* view_buf) const
{
	TriangleMeshView& view = *(TriangleMeshView*)view_buf;
	view.normalMat = m_norm_mat;
	view.color = { m_color, 1.0f };
	view.vertexBuf = m_vertexBuffer->get_device_address();
	view.indexBuf = m_indexBuffer->get_device_address();
	view.material = m_material;
	view.fuzz = m_fuzz;
	view.ref_idx = m_ref_idx;
}