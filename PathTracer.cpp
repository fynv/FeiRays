#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "context.h"
#include "PathTracer.h"
#include "shaders/common/bindings.h"
#include "RNGState_xorwow.h"
#include "RNGInitializer.h"
#include "SRGBConverter.h"
#include "Timing.h"

#ifndef PI
#define PI 3.1415926f
#endif

#include <gtc/matrix_transform.hpp>


Geometry::Geometry(const glm::mat4x4& model)
{
	m_model = model;
	m_norm_mat = glm::transpose(glm::inverse(model));

	m_blas = nullptr;
}

Geometry::~Geometry()
{
	delete m_blas;
}

static glm::mat4x4 s_sphere_model(const glm::vec3& center, float r)
{
	glm::mat4x4 ret = glm::identity<glm::mat4x4>();
	ret = glm::translate(ret, center);
	ret = glm::scale(ret, glm::vec3(r, r, r));
	return ret;
}

void Geometry::_blas_create_procedure(DeviceBuffer* aabb_buf)
{
	const Context& ctx = Context::get_context();

	VkAccelerationStructureGeometryKHR acceleration_geometry = {};
	acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
	acceleration_geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
	acceleration_geometry.geometry.aabbs.data.deviceAddress = aabb_buf->get_device_address();
	acceleration_geometry.geometry.aabbs.stride = 0;

	VkAccelerationStructureBuildGeometryInfoKHR geoBuildInfo{};
	geoBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	geoBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	geoBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	geoBuildInfo.geometryCount = 1;
	geoBuildInfo.pGeometries = &acceleration_geometry;

	VkAccelerationStructureBuildRangeInfoKHR range{};
	range.primitiveCount = 1;
	range.primitiveOffset = 0;
	range.firstVertex = 0;
	range.transformOffset = 0;
	const VkAccelerationStructureBuildRangeInfoKHR* ranges = &range;

	m_blas = new BaseLevelAS(geoBuildInfo, &acceleration_geometry, &ranges);
}

void Geometry::_blas_create_indexed_triangles(DeviceBuffer* positionBuffer, DeviceBuffer* indexBuffer)
{
	const Context& ctx = Context::get_context();

	VkAccelerationStructureGeometryKHR acceleration_geometry = {};
	acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	acceleration_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	acceleration_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	acceleration_geometry.geometry.triangles.vertexData.deviceAddress = positionBuffer->get_device_address();
	acceleration_geometry.geometry.triangles.maxVertex = (unsigned)(positionBuffer->size() / sizeof(glm::vec3));
	acceleration_geometry.geometry.triangles.vertexStride = sizeof(glm::vec3);
	acceleration_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	acceleration_geometry.geometry.triangles.indexData.deviceAddress = indexBuffer->get_device_address();

	VkAccelerationStructureBuildGeometryInfoKHR geoBuildInfo{};
	geoBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	geoBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	geoBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	geoBuildInfo.geometryCount = 1;
	geoBuildInfo.pGeometries = &acceleration_geometry;

	VkAccelerationStructureBuildRangeInfoKHR range{};
	range.primitiveCount = (unsigned)(indexBuffer->size() / sizeof(unsigned) / 3);
	range.primitiveOffset = 0;
	range.firstVertex = 0;
	range.transformOffset = 0;
	const VkAccelerationStructureBuildRangeInfoKHR* ranges = &range;

	m_blas = new BaseLevelAS(geoBuildInfo, &acceleration_geometry, &ranges);
}


SphereLight::SphereLight(const glm::vec3& center, float r, const glm::vec3& color) : Geometry(s_sphere_model(center, r))
{
	m_center = center;
	m_radius = r;
	m_color = color;
	static float s_aabb[6] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

	DeviceBuffer aabb_buf(sizeof(float) * 6, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
	aabb_buf.upload(s_aabb);
	_blas_create_procedure(&aabb_buf);
}

SphereLight::~SphereLight()
{

}

struct View_SphereLight
{
	glm::vec4 center_radius;
	glm::vec4 color;
};


GeoCls SphereLight::cls() const
{
	static const char s_name[] = "SphereLight";
	static const char s_fn_rint[] = "geometry/intersection_unit_spheres.spv";
	static const char s_fn_rchit[] = "geometry/closesthit_sphere_lights.spv";
	GeoCls cls = {};
	cls.name = s_name;
	cls.size_view = sizeof(View_SphereLight);
	cls.binding_view = BINDING_SphereLight;
	cls.fn_intersection = s_fn_rint;
	cls.fn_closesthit = s_fn_rchit;
	return cls;
}

void SphereLight::get_view(void* view_buf) const
{
	View_SphereLight& view = *(View_SphereLight*)view_buf;
	view.center_radius = glm::vec4(m_center, m_radius);
	view.color = glm::vec4(m_color, 1.0f);
}

struct View_GradientSky
{
	glm::vec4 color0;
	glm::vec4 color1;
};


GradientSky::GradientSky(const glm::vec3& color0, const glm::vec3& color1)
{
	m_color0 = color0;
	m_color1 = color1;
}

GradientSky::~GradientSky()
{

}


SkyCls GradientSky::cls() const
{
	static const char s_name[] = "common/miss.spv";
	SkyCls cls;
	cls.fn_missing = s_name;
	cls.size_view = sizeof(View_GradientSky);
	return cls;
}

void GradientSky::get_view(void* view_buf) const
{
	View_GradientSky& view = *(View_GradientSky*)view_buf;
	view.color0 = glm::vec4(m_color0, 1.0f);
	view.color1 = glm::vec4(m_color1, 1.0f);
}

RGBATexture::RGBATexture(int width, int height, void* data, bool srgb)
{
	if (srgb)
		m_data = new Texture(width, height, 4, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);
	else
		m_data = new Texture(width, height, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);

	m_data->uploadTexture(data);
}

RGBATexture::~RGBATexture()
{
	delete m_data;
}


RGBACubemap::RGBACubemap(int width, int height, void* data, bool srgb)
{
	if (srgb)
		m_data = new Cubemap(width, height, 4, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);
	else
		m_data = new Cubemap(width, height, 4, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);
	m_data->uploadTexture(data);
}

RGBACubemap::~RGBACubemap()
{
	delete m_data;
}


TexturedSkyBox::TexturedSkyBox(int texId)
{
	m_texId = texId;
	m_transform = glm::identity<glm::mat4x4>();
}

TexturedSkyBox::~TexturedSkyBox()
{

}

void TexturedSkyBox::rotate(float angle, const glm::vec3& v)
{
	m_transform = glm::rotate(m_transform, angle, v);
}

struct View_TexturedSkyBox
{
	glm::mat3x4 transform;
	int texIdx;
};

SkyCls TexturedSkyBox::cls() const
{
	static const char s_name[] = "common/miss_tex_sky.spv";
	SkyCls cls;
	cls.fn_missing = s_name;
	cls.size_view = sizeof(View_TexturedSkyBox);
	return cls;
}

void TexturedSkyBox::get_view(void* view_buf) const
{
	View_TexturedSkyBox& view = *(View_TexturedSkyBox*)view_buf;
	view.transform = m_transform;
	view.texIdx = m_texId;
}

Image::Image(int width, int height, float* hdata, int batch_size)
{
	m_width = width;
	m_height = height;
	if (batch_size > m_width*m_height)
	{
		m_batch_size = m_width * m_height;
	}
	else if (batch_size < 1024)
	{
		m_batch_size = 1024;
	}
	else
	{
		m_batch_size = batch_size;
	}

	if (m_batch_size > (1 << 18)) m_batch_size = 1 << 18;

	m_batch_size = (m_batch_size + 63) / 64 * 64; // 8x8 blocks

	m_data = new DeviceBuffer(sizeof(float) * 4 * width * height, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

	if (hdata != nullptr)
		m_data->upload(hdata);

	m_rng_states = nullptr;
}

Image::~Image()
{
	delete m_rng_states;
	delete m_data;
}


DeviceBuffer* Image::rng_states()
{
	if (m_rng_states == nullptr)
	{
		m_rng_states = new DeviceBuffer(sizeof(RNGState) * m_batch_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

		{
			printf("Initializing RNG states..\n");
			double time0 = GetTime();
			const RNGInitializer& initializer = RNGInitializer::get_initializer();
			initializer.InitRNGs(m_rng_states);
			double time1 = GetTime();
			printf("Done initializing RNG states.. %f secs\n", time1 - time0);
		}

	}

	return m_rng_states;
}


void Image::clear()
{
	m_data->zero();
}

void Image::to_host_raw(float *hdata) const
{
	m_data->download(hdata);
}


inline float clamp01(float f)
{
	float v = f;
	if (v < 0.0f) v = 0.0f;
	else if (v > 1.0f) v = 1.0f;
	return v;
}

void Image::to_host_srgb(unsigned char* hdata, float boost) const
{
	unsigned count = unsigned(m_width*m_height);
	const SRGBConverter& converter = SRGBConverter::get_converter();
	Texture colBuf(m_width, m_height, 4, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	converter.convert(&colBuf, m_data, boost);
	std::vector<unsigned char> raw(count * 4);
	colBuf.downloadTexture(raw.data());
	for (unsigned i = 0; i < count; i++)
	{
		const unsigned char* pIn = &raw[i * 4];
		unsigned char* pOut = hdata + i * 3;
		pOut[0] = pIn[0];
		pOut[1] = pIn[1];
		pOut[2] = pIn[2];
	}
}


PathTracer::PathTracer()
{
	m_target = nullptr;
	set_camera({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, 90.0f);

	m_Sampler = new Sampler;

	m_default_sky = new GradientSky;
	m_current_sky = m_default_sky;
}

PathTracer::~PathTracer()
{
	delete m_default_sky;
	delete m_Sampler;
}

void PathTracer::set_sky(Sky* sky)
{
	if (m_current_sky == m_default_sky)
	{
		delete m_default_sky;
		m_default_sky = nullptr;
	}
	m_current_sky = sky;
}

void PathTracer::add_geometry(Geometry* geo)
{
	GeoCls cls = geo->cls();
	auto iter = m_geo_lists.find(cls.name);
	if (iter == m_geo_lists.end())
		m_geo_lists[cls.name].cls = cls;
	m_geo_lists[cls.name].list.push_back(geo);
}

void PathTracer::add_sunlight(glm::vec3 direction, float radian, glm::vec3 color)
{
	m_sunlights.push_back({ { glm::normalize(direction), radian}, {color, 1.0f} });
}

int PathTracer::add_texture(RGBATexture* tex)
{
	int id = (int)m_textures.size();
	m_textures.push_back(tex);
	return id;
}

int PathTracer::add_cubemap(RGBACubemap* tex)
{
	int id = (int)m_cubemaps.size();
	m_cubemaps.push_back(tex);
	return id;
}

void PathTracer::set_camera(glm::vec3 lookfrom, glm::vec3 lookat, glm::vec3 vup, float vfov, float aperture, float focus_dist)
{
	m_lookfrom = lookfrom;
	m_lookat = lookat;
	m_vup = vup;
	m_vfov = vfov;
	m_aperture = aperture;
	m_focus_dist = focus_dist;
}

struct ImageView
{
	VkDeviceAddress data;
	int width;
	int height;
};

struct RayGenParams
{
	glm::vec4 origin;
	glm::vec4 upper_left;
	glm::vec4 ux;
	glm::vec4 uy;
	ImageView target;
	float lens_radius;
	int num_iter;
};

struct LightSourceDist
{
	VkDeviceAddress buf;
	int number_sphere_lights;
	int number_sun_lights;
};

struct RayTrace
{
	int num_iter;

	TopLevelAS* tlas;
	std::vector<DeviceBuffer*> buf_views;
	DeviceBuffer* params_raygen;
	DeviceBuffer* params_sky;

	DeviceBuffer* buf_light_source_dist;
	DeviceBuffer* light_source_dist;
	DeviceBuffer* sunlights;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;

	VkPipelineLayout rt_pipelineLayout;
	VkPipeline rt_pipeline;

	DeviceBuffer* shaderBindingTableBuffer;

	VkPipelineLayout comp_pipelineLayout;
	VkPipeline comp_pipeline;
};

void PathTracer::_tlas_create(RayTrace& rt) const
{
	size_t num_hitgroups = m_geo_lists.size();
	std::vector<size_t> num_instances(num_hitgroups);

	std::vector<std::vector<VkAccelerationStructureKHR>> blases(num_hitgroups);
	std::vector<const VkAccelerationStructureKHR*> pblases(num_hitgroups);

	std::vector<std::vector<glm::mat4x4>> transforms(num_hitgroups);
	std::vector<const glm::mat4x4*> ptransforms(num_hitgroups);

	size_t i = 0;
	auto iter = m_geo_lists.begin();
	while (iter != m_geo_lists.end())
	{
		const GeoCls& cls = iter->second.cls;
		const std::vector<Geometry*>& lst = iter->second.list;
		num_instances[i] = lst.size();

		blases[i].resize(num_instances[i]);
		pblases[i] = blases[i].data();

		transforms[i].resize(num_instances[i]);
		ptransforms[i] = transforms[i].data();

		for (size_t j = 0; j < num_instances[i]; j++)
		{
			blases[i][j] = lst[j]->get_blas().structure();
			transforms[i][j] = lst[j]->model();
		}

		iter++;
		i++;
	}

	rt.tlas = new TopLevelAS(num_hitgroups, num_instances.data(), pblases.data(), ptransforms.data());
}

void PathTracer::_args_create(RayTrace& rt) const
{
	const Context& ctx = Context::get_context();
	size_t num_hitgroups = m_geo_lists.size();
	SkyCls sky_cls = m_current_sky->cls();

	rt.buf_views.resize(num_hitgroups);

	size_t i = 0;
	auto iter = m_geo_lists.begin();
	while (iter != m_geo_lists.end())
	{
		const GeoCls& cls = iter->second.cls;
		const std::vector<Geometry*>& lst = iter->second.list;
		size_t num_instances = lst.size();

		std::vector<char> views(cls.size_view*num_instances);
		for (size_t j = 0; j < num_instances; j++)
			lst[j]->get_view(&views[cls.size_view*j]);

		rt.buf_views[i] = new DeviceBuffer(cls.size_view*num_instances, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		rt.buf_views[i]->upload(views.data());

		iter++;
		i++;
	}


	rt.params_raygen = new DeviceBuffer(sizeof(RayGenParams), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	if (sky_cls.size_view > 0)
	{
		rt.params_sky = new DeviceBuffer(sky_cls.size_view, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		std::vector<char> buf(sky_cls.size_view);
		m_current_sky->get_view(buf.data());
		rt.params_sky->upload(buf.data());
	}
	else
	{
		rt.params_sky = nullptr;
	}

	rt.light_source_dist = new DeviceBuffer(sizeof(LightSourceDist), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	int num_sphere_lights = 0;
	iter = m_geo_lists.find("SphereLight");
	if (iter != m_geo_lists.end())
	{
		num_sphere_lights = (int)iter->second.list.size();
	}
	int num_sunlights = (int)m_sunlights.size();
	int total_lights = num_sphere_lights + num_sunlights;

	if (total_lights > 0)
	{
		rt.buf_light_source_dist = new DeviceBuffer(sizeof(float)*total_lights, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	}
	else
	{
		rt.buf_light_source_dist = nullptr;
	}

	if (m_sunlights.size() > 0)
	{
		rt.sunlights = new DeviceBuffer(sizeof(Sunlight) * m_sunlights.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		rt.sunlights->upload(m_sunlights.data());
	}
	else
	{
		rt.sunlights = nullptr;
	}

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(BINDING_START + num_hitgroups);

	descriptorSetLayoutBindings[0] = {};
	descriptorSetLayoutBindings[0].binding = 0;
	descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	descriptorSetLayoutBindings[0].descriptorCount = 1;
	descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	descriptorSetLayoutBindings[1] = {};
	descriptorSetLayoutBindings[1].binding = 1;
	descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBindings[1].descriptorCount = 1;
	descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_COMPUTE_BIT;
	descriptorSetLayoutBindings[2] = {};
	descriptorSetLayoutBindings[2].binding = 2;
	descriptorSetLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBindings[2].descriptorCount = 1;
	descriptorSetLayoutBindings[2].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	descriptorSetLayoutBindings[3] = {};
	descriptorSetLayoutBindings[3].binding = 3;
	descriptorSetLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetLayoutBindings[3].descriptorCount = (uint32_t)(m_textures.size());
	descriptorSetLayoutBindings[3].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	descriptorSetLayoutBindings[4] = {};
	descriptorSetLayoutBindings[4].binding = 4;
	descriptorSetLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetLayoutBindings[4].descriptorCount = (uint32_t)(m_cubemaps.size());
	descriptorSetLayoutBindings[4].stageFlags = VK_SHADER_STAGE_MISS_BIT_KHR;
	descriptorSetLayoutBindings[5] = {};
	descriptorSetLayoutBindings[5].binding = 5;
	descriptorSetLayoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBindings[5].descriptorCount = 1;
	descriptorSetLayoutBindings[5].stageFlags = VK_SHADER_STAGE_MISS_BIT_KHR;
	descriptorSetLayoutBindings[6] = {};
	descriptorSetLayoutBindings[6].binding = 6;
	descriptorSetLayoutBindings[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBindings[6].descriptorCount = 1;
	descriptorSetLayoutBindings[6].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
	descriptorSetLayoutBindings[7] = {};
	descriptorSetLayoutBindings[7].binding = 7;
	descriptorSetLayoutBindings[7].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorSetLayoutBindings[7].descriptorCount = 1;
	descriptorSetLayoutBindings[7].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;


	i = 0;
	iter = m_geo_lists.begin();
	while (iter != m_geo_lists.end())
	{
		descriptorSetLayoutBindings[BINDING_START + i] = {};
		descriptorSetLayoutBindings[BINDING_START + i].binding = iter->second.cls.binding_view;
		descriptorSetLayoutBindings[BINDING_START + i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorSetLayoutBindings[BINDING_START + i].descriptorCount = 1;
		descriptorSetLayoutBindings[BINDING_START + i].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		if (iter->first == "SphereLight")
			descriptorSetLayoutBindings[BINDING_START + i].stageFlags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		iter++;
		i++;
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)descriptorSetLayoutBindings.size();
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	vkCreateDescriptorSetLayout(ctx.device(), &descriptorSetLayoutCreateInfo, nullptr, &rt.descriptorSetLayout);

	VkDescriptorPoolSize descriptorPoolSize[4];
	descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	descriptorPoolSize[0].descriptorCount = 1;
	descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize[1].descriptorCount = 3;
	descriptorPoolSize[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize[2].descriptorCount = (uint32_t)(2 + num_hitgroups);
	descriptorPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSize[3].descriptorCount = (uint32_t)(m_textures.size() + m_cubemaps.size());

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSize[3].descriptorCount > 0 ? 4 : 3;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize;
	vkCreateDescriptorPool(ctx.device(), &descriptorPoolCreateInfo, nullptr, &rt.descriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = rt.descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &rt.descriptorSetLayout;

	vkAllocateDescriptorSets(ctx.device(), &descriptorSetAllocateInfo, &rt.descriptorSet);

	VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo = {};
	descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
	descriptorAccelerationStructureInfo.pAccelerationStructures = &rt.tlas->structure();

	VkDescriptorBufferInfo descriptorBufferInfo_raygen = {};
	descriptorBufferInfo_raygen.buffer = rt.params_raygen->buf();
	descriptorBufferInfo_raygen.range = VK_WHOLE_SIZE;

	VkDescriptorBufferInfo descriptorBufferInfo_rng_states = {};
	descriptorBufferInfo_rng_states.buffer = m_target->rng_states()->buf();
	descriptorBufferInfo_rng_states.range = VK_WHOLE_SIZE;

	std::vector<VkDescriptorImageInfo> imageInfos(m_textures.size());
	for (size_t i = 0; i < m_textures.size(); ++i)
	{
		imageInfos[i] = {};
		imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[i].imageView = m_textures[i]->data()->view();
		imageInfos[i].sampler = m_Sampler->sampler();
	}

	std::vector<VkDescriptorImageInfo> cubemapInfos(m_cubemaps.size());
	for (size_t i = 0; i < m_cubemaps.size(); ++i)
	{
		cubemapInfos[i] = {};
		cubemapInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		cubemapInfos[i].imageView = m_cubemaps[i]->data()->view();
		cubemapInfos[i].sampler = m_Sampler->sampler();
	}

	VkDescriptorBufferInfo descriptorBufferInfo_sky = {};
	if (rt.params_sky != nullptr)
		descriptorBufferInfo_sky.buffer = rt.params_sky->buf();
	descriptorBufferInfo_sky.range = VK_WHOLE_SIZE;

	VkDescriptorBufferInfo descriptorBufferInfo_lightsource_dist = {};
	descriptorBufferInfo_lightsource_dist.buffer = rt.light_source_dist->buf();
	descriptorBufferInfo_lightsource_dist.range = VK_WHOLE_SIZE;

	VkDescriptorBufferInfo descriptorBufferInfo_sunlight = {};
	if (rt.sunlights != nullptr)
		descriptorBufferInfo_sunlight.buffer = rt.sunlights->buf();
	descriptorBufferInfo_sunlight.range = VK_WHOLE_SIZE;

	std::vector<VkDescriptorBufferInfo> descriptorBufferInfo_geo_views(num_hitgroups);
	for (i = 0; i < num_hitgroups; i++)
	{
		descriptorBufferInfo_geo_views[i].buffer = rt.buf_views[i]->buf();
		descriptorBufferInfo_geo_views[i].range = VK_WHOLE_SIZE;
	}

	std::vector<VkWriteDescriptorSet> writeDescriptorSet(3 + num_hitgroups);

	writeDescriptorSet[0] = {};
	writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[0].dstSet = rt.descriptorSet;
	writeDescriptorSet[0].dstBinding = 0;
	writeDescriptorSet[0].descriptorCount = 1;
	writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	writeDescriptorSet[0].pNext = &descriptorAccelerationStructureInfo;

	writeDescriptorSet[1] = {};
	writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[1].dstSet = rt.descriptorSet;
	writeDescriptorSet[1].dstBinding = 1;
	writeDescriptorSet[1].descriptorCount = 1;
	writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet[1].pBufferInfo = &descriptorBufferInfo_raygen;

	writeDescriptorSet[2] = {};
	writeDescriptorSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[2].dstSet = rt.descriptorSet;
	writeDescriptorSet[2].dstBinding = 2;
	writeDescriptorSet[2].descriptorCount = 1;
	writeDescriptorSet[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSet[2].pBufferInfo = &descriptorBufferInfo_rng_states;

	for (i = 0; i < num_hitgroups; i++)
	{
		writeDescriptorSet[3 + i] = {};
		writeDescriptorSet[3 + i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[3 + i].dstSet = rt.descriptorSet;
		writeDescriptorSet[3 + i].dstBinding = descriptorSetLayoutBindings[BINDING_START + i].binding;
		writeDescriptorSet[3 + i].descriptorCount = 1;
		writeDescriptorSet[3 + i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writeDescriptorSet[3 + i].pBufferInfo = &descriptorBufferInfo_geo_views[i];
	}

	if (m_textures.size() > 0)
	{
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = rt.descriptorSet;
		wds.dstBinding = 3;
		wds.descriptorCount = (uint32_t)(m_textures.size());
		wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wds.pImageInfo = imageInfos.data();
		writeDescriptorSet.push_back(wds);
	}

	if (m_cubemaps.size() > 0)
	{
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = rt.descriptorSet;
		wds.dstBinding = 4;
		wds.descriptorCount = (uint32_t)(m_cubemaps.size());
		wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wds.pImageInfo = cubemapInfos.data();
		writeDescriptorSet.push_back(wds);
	}

	if (rt.params_sky != nullptr)
	{
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = rt.descriptorSet;
		wds.dstBinding = 5;
		wds.descriptorCount = 1;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		wds.pBufferInfo = &descriptorBufferInfo_sky;
		writeDescriptorSet.push_back(wds);
	}

	{
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = rt.descriptorSet;
		wds.dstBinding = 6;
		wds.descriptorCount = 1;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		wds.pBufferInfo = &descriptorBufferInfo_lightsource_dist;
		writeDescriptorSet.push_back(wds);
	}

	if (rt.sunlights != nullptr)
	{
		VkWriteDescriptorSet wds = {};
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = rt.descriptorSet;
		wds.dstBinding = 7;
		wds.descriptorCount = 1;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		wds.pBufferInfo = &descriptorBufferInfo_sunlight;
		writeDescriptorSet.push_back(wds);
	}

	vkUpdateDescriptorSets(ctx.device(), (uint32_t)writeDescriptorSet.size(), writeDescriptorSet.data(), 0, nullptr);

}

void PathTracer::_rt_pipeline_create(RayTrace& rt) const
{
	const Context& ctx = Context::get_context();
	size_t num_hitgroups = m_geo_lists.size();
	SkyCls sky_cls = m_current_sky->cls();

	VkShaderModule rayGenModule = ctx.get_shader("path_tracer/raygen.spv");
	VkShaderModule missModule = ctx.get_shader(sky_cls.fn_missing);

	std::vector<VkShaderModule> intersection_modules(num_hitgroups);
	std::vector<VkShaderModule> closesthit_modules(num_hitgroups);

	size_t i = 0;
	size_t count_intersection = 0;
	auto iter = m_geo_lists.begin();
	while (iter != m_geo_lists.end())
	{
		if (iter->second.cls.fn_intersection != nullptr)
		{
			intersection_modules[i] = ctx.get_shader(iter->second.cls.fn_intersection);
			count_intersection++;
		}
		else
		{
			intersection_modules[i] = nullptr;
		}
		closesthit_modules[i] = ctx.get_shader(iter->second.cls.fn_closesthit);

		iter++;
		i++;
	}

	size_t stage_count = 2 + count_intersection + num_hitgroups;
	size_t group_count = 2 + num_hitgroups;

	static const char s_main[] = "main";

	std::vector<VkPipelineShaderStageCreateInfo> stages(stage_count);

	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	stages[0].module = rayGenModule;
	stages[0].pName = s_main;

	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	stages[1].module = missModule;
	stages[1].pName = s_main;

	int i_stages = 2;
	for (size_t i = 0; i < num_hitgroups; i++)
	{
		if (intersection_modules[i] != nullptr)
		{
			stages[i_stages].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stages[i_stages].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
			stages[i_stages].module = intersection_modules[i];
			stages[i_stages].pName = s_main;
			i_stages++;
		}
		stages[i_stages].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[i_stages].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		stages[i_stages].module = closesthit_modules[i];
		stages[i_stages].pName = s_main;
		i_stages++;
	}

	std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups(group_count);

	groups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	groups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	groups[0].generalShader = 0;
	groups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
	groups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
	groups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

	groups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	groups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	groups[1].generalShader = 1;
	groups[1].closestHitShader = VK_SHADER_UNUSED_KHR;
	groups[1].anyHitShader = VK_SHADER_UNUSED_KHR;
	groups[1].intersectionShader = VK_SHADER_UNUSED_KHR;

	i_stages = 2;
	for (size_t i = 0; i < num_hitgroups; i++)
	{
		if (intersection_modules[i] == nullptr)
		{
			groups[2 + i].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			groups[2 + i].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			groups[2 + i].generalShader = VK_SHADER_UNUSED_KHR;
			groups[2 + i].closestHitShader = i_stages++;
			groups[2 + i].anyHitShader = VK_SHADER_UNUSED_KHR;
			groups[2 + i].intersectionShader = VK_SHADER_UNUSED_KHR;
		}
		else
		{
			groups[2 + i].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			groups[2 + i].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
			groups[2 + i].generalShader = VK_SHADER_UNUSED_KHR;
			groups[2 + i].intersectionShader = i_stages++;
			groups[2 + i].closestHitShader = i_stages++;
			groups[2 + i].anyHitShader = VK_SHADER_UNUSED_KHR;
		}
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &rt.descriptorSetLayout;

	vkCreatePipelineLayout(ctx.device(), &pipelineLayoutCreateInfo, nullptr, &rt.rt_pipelineLayout);

	VkRayTracingPipelineCreateInfoKHR rayPipelineInfo = {};
	rayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayPipelineInfo.stageCount = (uint32_t)stage_count;
	rayPipelineInfo.pStages = stages.data();
	rayPipelineInfo.groupCount = (uint32_t)group_count;
	rayPipelineInfo.pGroups = groups.data();
	rayPipelineInfo.maxPipelineRayRecursionDepth = 1;
	rayPipelineInfo.layout = rt.rt_pipelineLayout;	

	vkCreateRayTracingPipelinesKHR(ctx.device(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayPipelineInfo, nullptr, &rt.rt_pipeline);

	// shader binding table
	unsigned progIdSize = ctx.raytracing_properties().shaderGroupHandleSize;
	unsigned baseAlignment = ctx.raytracing_properties().shaderGroupBaseAlignment;
	unsigned sbtSize_get = progIdSize * (unsigned)group_count;
	unsigned sbtSize = baseAlignment * (unsigned)group_count;

	rt.shaderBindingTableBuffer = new DeviceBuffer(sbtSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

	unsigned char* shaderHandleStorage_get = (unsigned char*)malloc(sbtSize_get);
	unsigned char* shaderHandleStorage = (unsigned char*)malloc(sbtSize);
	vkGetRayTracingShaderGroupHandlesKHR(ctx.device(), rt.rt_pipeline, 0, (uint32_t)group_count, sbtSize_get, shaderHandleStorage_get);
	for (unsigned i = 0; i < group_count; i++)
		memcpy(shaderHandleStorage + i * baseAlignment, shaderHandleStorage_get + i * progIdSize, progIdSize);

	rt.shaderBindingTableBuffer->upload(shaderHandleStorage);
	free(shaderHandleStorage);
	free(shaderHandleStorage_get);
}

void PathTracer::_comp_pipeline_create(RayTrace& rt) const
{
	static const char s_main[] = "main";

	const Context& ctx = Context::get_context();
	VkShaderModule finalModule = ctx.get_shader("path_tracer/final.spv");

	VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
	computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageInfo.module = finalModule;
	computeShaderStageInfo.pName = s_main;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &rt.descriptorSetLayout;

	vkCreatePipelineLayout(ctx.device(), &pipelineLayoutCreateInfo, 0, &rt.comp_pipelineLayout);

	VkComputePipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage = computeShaderStageInfo;
	pipelineInfo.layout = rt.comp_pipelineLayout;

	vkCreateComputePipelines(ctx.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &rt.comp_pipeline);
}

void PathTracer::_calc_raygen(RayTrace& rt) const
{
	glm::vec3 origin = m_lookfrom;

	float theta = m_vfov * PI / 180.0f;
	float half_height = tanf(theta*0.5f)*m_focus_dist;
	float size_pix = half_height * 2.0f / (float)m_target->height();
	float half_width = size_pix * (float)m_target->width()*0.5f;

	glm::vec3 axis_z = normalize(m_lookfrom - m_lookat);
	glm::vec3 axis_x = normalize(cross(m_vup, axis_z));
	glm::vec3 axis_y = cross(axis_z, axis_x);

	glm::vec3 plane_center = m_lookfrom - axis_z * m_focus_dist;
	glm::vec3 upper_left = plane_center - axis_x * half_width + axis_y * half_height;
	glm::vec3 ux = size_pix * axis_x;
	glm::vec3 uy = -size_pix * axis_y;

	RayGenParams raygen_params;
	ImageView target_view;
	{
		target_view.data = m_target->data()->get_device_address();
		target_view.width = m_target->width();
		target_view.height = m_target->height();
	}
	raygen_params.target = target_view;
	raygen_params.origin = glm::vec4(origin, 1.0f);
	raygen_params.upper_left = glm::vec4(upper_left, 1.0f);
	raygen_params.ux = glm::vec4(ux, 1.0f);
	raygen_params.uy = glm::vec4(uy, 1.0f);
	raygen_params.lens_radius = m_aperture * 0.5f;
	raygen_params.num_iter = rt.num_iter;

	rt.params_raygen->upload(&raygen_params);
}

void PathTracer::_calc_light_source_dist(RayTrace& rt) const
{
	LightSourceDist lsd;
	lsd.number_sphere_lights = 0;

	auto iter = m_geo_lists.find("SphereLight");
	if (iter != m_geo_lists.end())
	{
		lsd.number_sphere_lights = (int)iter->second.list.size();
	}
	lsd.number_sun_lights = (int)m_sunlights.size();
	std::vector<float> h_dist(lsd.number_sphere_lights + lsd.number_sun_lights);

	if (h_dist.size() > 0)
	{
		float p1 = (float)lsd.number_sphere_lights / (float)h_dist.size();
		float p2 = (float)lsd.number_sun_lights / (float)h_dist.size();

		float total1 = 0.0f;

		// sphere-lights
		for (int i = 0; i < lsd.number_sphere_lights; i++)
		{
			SphereLight* sl = (SphereLight*)iter->second.list[i];
			glm::vec3 co = sl->color();
			float r = sl->radius();
			h_dist[i] = (co[0] + co[1] + co[2])*r*r;
			total1 += h_dist[i];
		}

		// sun-lights
		float total2 = 0.0f;
		for (int i = 0; i < lsd.number_sun_lights; i++)
		{
			glm::vec3 co = m_sunlights[i].color;
			float r = m_sunlights[i].dir_radian[3];
			int j = i + lsd.number_sphere_lights;
			h_dist[j] = (co[0] + co[1] + co[2])*r*r;
			total2 += h_dist[j];
		}

		for (int i = 0; i < lsd.number_sphere_lights; i++)
		{
			h_dist[i] *= p1 / total1;
			if (i > 0) h_dist[i] += h_dist[i - 1];
		}

		for (int i = 0; i < lsd.number_sun_lights; i++)
		{
			int j = i + lsd.number_sphere_lights;
			h_dist[j] *= p2 / total2;
			if (j > 0) h_dist[j] += h_dist[j - 1];
		}

		rt.buf_light_source_dist->upload(h_dist.data());
		lsd.buf = rt.buf_light_source_dist->get_device_address();
	}
	rt.light_source_dist->upload(&lsd);

}


void PathTracer::_rt_clean(RayTrace& rt) const
{
	const Context& ctx = Context::get_context();

	vkDestroyPipelineLayout(ctx.device(), rt.comp_pipelineLayout, nullptr);
	vkDestroyPipeline(ctx.device(), rt.comp_pipeline, nullptr);

	delete rt.shaderBindingTableBuffer;
	vkDestroyPipelineLayout(ctx.device(), rt.rt_pipelineLayout, nullptr);
	vkDestroyPipeline(ctx.device(), rt.rt_pipeline, nullptr);

	vkDestroyDescriptorPool(ctx.device(), rt.descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(ctx.device(), rt.descriptorSetLayout, nullptr);

	delete rt.sunlights;
	delete rt.light_source_dist;
	delete rt.buf_light_source_dist;
	delete rt.params_sky;
	delete rt.params_raygen;

	for (size_t i = 0; i < m_geo_lists.size(); i++)
		delete rt.buf_views[i];

	delete rt.tlas;
}


void PathTracer::trace(int num_iter, int interval) const
{
	if (m_target == nullptr) return;
	if (m_geo_lists.size() == 0) return;

	// trigger rng-state initialization, for timing
	m_target->rng_states();

	RayTrace rt;
	rt.num_iter = num_iter;

	printf("Preparing ray-tracing..\n");
	double time0 = GetTime();

	_tlas_create(rt);
	_args_create(rt);
	_rt_pipeline_create(rt);
	_comp_pipeline_create(rt);
	_calc_raygen(rt);
	_calc_light_source_dist(rt);


	double time1 = GetTime();
	printf("Done preparing ray-tracing.. %f secs\n", time1 - time0);

	m_target->clear();

	const Context& ctx = Context::get_context();
	unsigned baseAlignment = ctx.raytracing_properties().shaderGroupBaseAlignment;
	size_t num_hitgroups = m_geo_lists.size();

	VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry{};
	raygen_shader_sbt_entry.deviceAddress = rt.shaderBindingTableBuffer->get_device_address();
	raygen_shader_sbt_entry.stride = baseAlignment;
	raygen_shader_sbt_entry.size = baseAlignment;

	VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry{};
	miss_shader_sbt_entry.deviceAddress = raygen_shader_sbt_entry.deviceAddress + baseAlignment;
	miss_shader_sbt_entry.stride = baseAlignment;
	miss_shader_sbt_entry.size = baseAlignment;

	VkStridedDeviceAddressRegionKHR hit_shader_sbt_entry{};
	hit_shader_sbt_entry.deviceAddress = miss_shader_sbt_entry.deviceAddress + baseAlignment;
	hit_shader_sbt_entry.stride = baseAlignment;
	hit_shader_sbt_entry.size = baseAlignment * num_hitgroups;

	VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry{};

	if (interval == -1) interval = num_iter;

	printf("Doing ray-tracing..\n");
	double time2 = GetTime();

	int i = 0;
	while (i < num_iter)
	{
		int end = i + interval;
		if (end > num_iter) end = num_iter;

		{
			NTimeCommandBuffer cmdBuf(end - i);
			vkCmdBindPipeline(cmdBuf.buf(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt.rt_pipeline);
			vkCmdBindDescriptorSets(cmdBuf.buf(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt.rt_pipelineLayout, 0, 1, &rt.descriptorSet, 0, nullptr);
			vkCmdTraceRaysKHR(cmdBuf.buf(), &raygen_shader_sbt_entry, &miss_shader_sbt_entry, &hit_shader_sbt_entry, &callable_shader_sbt_entry, m_target->batch_size(), 1, 1);

			VkMemoryBarrier memoryBarrier = {};
			memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(cmdBuf.buf(), VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

			i = end;
		}

		if (i < num_iter)
			printf("%.2f%%\n", (float)i / (float)num_iter*100.0f);
	}

	{
		NTimeCommandBuffer cmdBuf;
		int group_x = (m_target->width() + 15) / 16;
		int group_y = (m_target->height() + 15) / 16;
		{
			vkCmdBindPipeline(cmdBuf.buf(), VK_PIPELINE_BIND_POINT_COMPUTE, rt.comp_pipeline);
			vkCmdBindDescriptorSets(cmdBuf.buf(), VK_PIPELINE_BIND_POINT_COMPUTE, rt.comp_pipelineLayout, 0, 1, &rt.descriptorSet, 0, nullptr);
			vkCmdDispatch(cmdBuf.buf(), group_x, group_y, 1);
		}
	}

	double time3 = GetTime();
	printf("Done ray-tracing.. %f secs\n", time3 - time2);

	_rt_clean(rt);
}
