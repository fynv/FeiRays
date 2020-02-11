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
	const VkPhysicalDeviceRayTracingPropertiesNV& raytracing_properties()  const { return m_raytracingProperties; }
	const VkDevice& device() const { return m_device; }
	const VkQueue& queue() const { return m_graphicsQueue; }
	const VkCommandPool& commandPool() const { return m_commandPool_graphics; }

private:
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkPhysicalDeviceBufferDeviceAddressFeaturesEXT m_bufferDeviceAddressFeatures;
	VkPhysicalDeviceFeatures2 m_features2;
	VkPhysicalDeviceRayTracingPropertiesNV m_raytracingProperties;
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
	
	uint64_t get_device_address() const;

protected:
	VkDeviceSize m_size;
	VkBuffer m_buf;
	VkDeviceMemory m_mem;


	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, bool ext_mem);
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

	void download(void* hdata);
};

class DeviceBuffer : public Buffer
{
public:
	DeviceBuffer(VkDeviceSize size, VkBufferUsageFlags usage, bool ext_mem = false);
	virtual ~DeviceBuffer();

	void upload(const void* hdata);
	void zero();
	void download(void* hdata, VkDeviceSize begin = 0, VkDeviceSize end = (VkDeviceSize)(-1));

};

class AS
{
public:
	const VkAccelerationStructureNV& structure() const { return m_structure; }

protected:
	AS();
	virtual ~AS();
	VkAccelerationStructureNV m_structure;
};

class BaseLevelAS : public AS
{
public:
	BaseLevelAS(uint32_t geometryCount, const VkGeometryNV* pGeometries);
	virtual ~BaseLevelAS();

private:
	DeviceBuffer* m_scratchBuffer;
	DeviceBuffer* m_resultBuffer;
};

class TopLevelAS : public AS
{
public:
	TopLevelAS(size_t num_hitgroups, size_t* num_instances, const VkAccelerationStructureNV** pblases, const glm::mat4x4** ptransforms);
	virtual ~TopLevelAS();

private:
	DeviceBuffer* m_scratchBuffer;
	DeviceBuffer* m_resultBuffer;
	DeviceBuffer* m_instancesBuffer;

};
