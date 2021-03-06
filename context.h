#pragma once

#include "volk.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <glm.hpp>

class Context
{
public:
	static const Context& get_context();

	const VkInstance& instance() const { return m_instance; }
	const VkPhysicalDevice& physicalDevice() const { return m_physicalDevice; }
	const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& raytracing_properties()  const { return m_rayTracingPipelineProperties; }
	const VkDevice& device() const { return m_device; }
	const VkQueue& queue() const { return m_graphicsQueue; }
	const VkCommandPool& commandPool() const { return m_commandPool_graphics; }

	VkShaderModule get_shader(const char* name) const;
private:
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

	VkPhysicalDeviceBufferDeviceAddressFeatures m_bufferDeviceAddressFeatures{};
	VkPhysicalDeviceDescriptorIndexingFeatures m_descriptorIndexingFeatures{};

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rayTracingPipelineFeatures{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures{};
	
	VkPhysicalDeviceScalarBlockLayoutFeatures m_scalarBlockLayoutFeatures{};
	VkPhysicalDeviceFeatures2 m_features2{};

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR  m_rayTracingPipelineProperties{};

	uint32_t m_graphicsQueueFamily;
	float m_queuePriority;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkCommandPool m_commandPool_graphics;


	bool _init_vulkan();
	Context();
	~Context();

};


class NTimeCommandBuffer
{
public:
	const VkCommandBuffer& buf() const { return m_buf; }

	NTimeCommandBuffer(size_t n = 1);
	~NTimeCommandBuffer();

private:
	VkCommandBuffer m_buf;
	size_t m_n;
};

class Buffer
{
public:
	VkDeviceSize size() const { return m_size; }
	const VkBuffer& buf() const { return m_buf; }
	const VkDeviceMemory& memory() const { return m_mem; }

	VkDeviceAddress get_device_address() const;

protected:
	VkDeviceSize m_size;
	VkBuffer m_buf;
	VkDeviceMemory m_mem;


	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
	virtual ~Buffer();

};

class UploadBuffer : public Buffer
{
public:
	UploadBuffer(VkDeviceSize size);
	virtual ~UploadBuffer();

	void upload(const void* hdata);
	void zero();

};

class DownloadBuffer : public Buffer
{
public:
	DownloadBuffer(VkDeviceSize size);
	virtual ~DownloadBuffer();

	void download(void* hdata) const;
};

class DeviceBuffer : public Buffer
{
public:
	DeviceBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
	virtual ~DeviceBuffer();

	void upload(const void* hdata);
	void zero();
	void download(void* hdata, VkDeviceSize begin = 0, VkDeviceSize end = (VkDeviceSize)(-1)) const;

};

class AS
{
public:
	const VkAccelerationStructureKHR & structure() const { return m_structure; }

protected:
	AS();
	virtual ~AS();
	VkAccelerationStructureKHR  m_structure;
};

class BaseLevelAS : public AS
{
public:
	BaseLevelAS(
		const VkAccelerationStructureBuildGeometryInfoKHR& geoBuildInfo,
		const VkAccelerationStructureGeometryKHR* pGeometries,
		const VkAccelerationStructureBuildRangeInfoKHR** ranges);

	virtual ~BaseLevelAS();

private:
	DeviceBuffer* m_scratchBuffer;
	DeviceBuffer* m_resultBuffer;
};


class TopLevelAS : public AS
{
public:
	TopLevelAS(size_t num_hitgroups, size_t* num_instances, const VkAccelerationStructureKHR** pblases, const glm::mat4x4** ptransforms);
	virtual ~TopLevelAS();

private:
	DeviceBuffer* m_scratchBuffer;
	DeviceBuffer* m_resultBuffer;
	DeviceBuffer* m_instancesBuffer;
};

class Texture
{
public:
	int width() const { return m_width; }
	int height() const { return m_height; }
	int pixel_size() const { return m_pixel_size; }
	const VkFormat& format() const { return m_format; }
	const VkImage& image() const { return m_image; }
	const VkDeviceMemory& memory() const { return m_mem; }
	const VkImageView& view() const { return m_view; }

	Texture(int width, int height, int pixel_size, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage);
	~Texture();

	void uploadTexture(const void* hdata);
	void downloadTexture(void* hdata) const;

private:
	int m_width;
	int m_height;
	int m_pixel_size;
	VkFormat m_format;

	VkImage m_image;
	VkDeviceMemory m_mem;
	VkImageView m_view;

};

class Cubemap
{
public:
	int width() const { return m_width; }
	int height() const { return m_height; }
	int pixel_size() const { return m_pixel_size; }
	const VkFormat& format() const { return m_format; }
	const VkImage& image() const { return m_image; }
	const VkDeviceMemory& memory() const { return m_mem; }
	const VkImageView& view() const { return m_view; }

	Cubemap(int width, int height, int pixel_size, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage);
	~Cubemap();

	void uploadTexture(const void* hdata);

private:
	int m_width;
	int m_height;
	int m_pixel_size;
	VkFormat m_format;

	VkImage m_image;
	VkDeviceMemory m_mem;
	VkImageView m_view;
};


class Sampler
{
public:
	Sampler();
	~Sampler();

	const VkSampler& sampler() const { return m_sampler; }

private:
	VkSampler m_sampler;
};

