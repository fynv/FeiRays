#include "context.h"
#include "shaders_packed.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string>
#include <unordered_map>

const Context& Context::get_context()
{
	static Context ctx;
	return ctx;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	printf("validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}

bool Context::_init_vulkan()
{
	if (volkInitialize() != VK_SUCCESS) return false;
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "FeiRays";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		const char* name_extensions[] = {
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		char str_validationLayers[] = "VK_LAYER_KHRONOS_validation";
		const char* validationLayers[] = { str_validationLayers };

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = sizeof(name_extensions)/sizeof(const char*);
		createInfo.ppEnabledExtensionNames = name_extensions;

#ifdef _DEBUG
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = validationLayers;	
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) return false;

#ifdef _DEBUG
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
		vkCreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger);
#endif
	}
	volkLoadInstance(m_instance);

	m_physicalDevice = VK_NULL_HANDLE;
	{
		// select physical device
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
		std::vector<VkPhysicalDevice> ph_devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, ph_devices.data());
		m_physicalDevice = ph_devices[0];
	}

	m_bufferDeviceAddressFeatures = {};
	m_descriptorIndexingFeatures = {};
	m_raytracingFeatures = {};
	{
		m_bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		m_bufferDeviceAddressFeatures.pNext = &m_descriptorIndexingFeatures;
		m_descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
		m_descriptorIndexingFeatures.pNext = &m_raytracingFeatures;
		m_raytracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR;
		m_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		m_features2.pNext = &m_bufferDeviceAddressFeatures;
		m_features2.features = {};
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_features2);
	}

	m_raytracingProperties = {};
	{
		m_raytracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_KHR;
		VkPhysicalDeviceProperties2 props = {};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props.pNext = &m_raytracingProperties;
		vkGetPhysicalDeviceProperties2(m_physicalDevice, &props);
	}

	m_graphicsQueueFamily = (uint32_t)(-1);
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilyCount; i++)
			if (m_graphicsQueueFamily == (uint32_t)(-1) && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) m_graphicsQueueFamily = i;
	}

	// logical device/queue
	m_queuePriority = 1.0f;

	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &m_queuePriority;

		const char* name_extensions[] = {
			VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME			
		};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.enabledExtensionCount = sizeof(name_extensions)/sizeof(const char*);
		createInfo.ppEnabledExtensionNames = name_extensions;
		createInfo.pNext = &m_features2;

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) return false;
	}

	vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);

	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_graphicsQueueFamily;
		vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool_graphics);
	}

	return true;

}

Context::Context()
{
	if (!_init_vulkan()) exit(0);
}

Context::~Context()
{
	/*
#ifdef _DEBUG
	vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
#endif
	vkDestroyCommandPool(m_device, m_commandPool_graphics, nullptr);
	vkDestroyDevice(m_device, nullptr);
	vkDestroyInstance(m_instance, nullptr);*/
}

VkShaderModule Context::get_shader(const char* name) const
{
	static std::unordered_map<std::string, int> s_name2spv;
	static std::unordered_map<std::string, VkShaderModule> s_name2module;
	static bool initialized = false;

	if (!initialized)
	{
		for (int i = 0; i < s_num_shaders; i++)
			s_name2spv[s_name_shaders[i]] = i;
		initialized = true;
	}
	auto iter = s_name2module.find(name);
	if (iter != s_name2module.end()) return iter->second;

	int size;
	const char* spv;
	std::vector<char> buf;

	auto iter2 = s_name2spv.find(name);
	if (iter2 != s_name2spv.end())
	{
		size = s_size_shaders[iter2->second];
		spv = s_content_shaders[iter2->second];
	}
	else
	{
		std::string fullname = "../shaders/";
		fullname += name;

		FILE* fp = fopen(fullname.data(), "rb");
		fseek(fp, 0, SEEK_END);
		size = (int)ftell(fp);
		fseek(fp, 0, SEEK_SET);		
		buf.resize(size);
		fread(buf.data(), 1, size, fp);
		fclose(fp);

		spv = buf.data();
	}

	const Context& ctx = Context::get_context();

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(spv);
	VkShaderModule shaderModule;
	vkCreateShaderModule(ctx.device(), &createInfo, nullptr, &shaderModule);

	s_name2module[name] = shaderModule;

	return shaderModule;
}

NTimeCommandBuffer::NTimeCommandBuffer(size_t n) : m_n(n)
{
	const Context& ctx = Context::get_context();

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = ctx.commandPool();
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(ctx.device(), &allocInfo, &m_buf);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (n == 1)
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	else
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(m_buf, &beginInfo);
}

NTimeCommandBuffer::~NTimeCommandBuffer()
{
	const Context& ctx = Context::get_context();

	vkEndCommandBuffer(m_buf);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_buf;

	for (size_t i = 0; i< m_n; i++)
		vkQueueSubmit(ctx.queue(), 1, &submitInfo, 0);

	vkQueueWaitIdle(ctx.queue());

	vkFreeCommandBuffers(ctx.device(), ctx.commandPool(), 1, &m_buf);
}

VkDeviceAddress Buffer::get_device_address() const
{
	if (m_size == 0) return 0;
	const Context& ctx = Context::get_context();

	VkBufferDeviceAddressInfo bufAdrInfo = {};
	bufAdrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufAdrInfo.buffer = m_buf;
	return vkGetBufferDeviceAddressKHR(ctx.device(), &bufAdrInfo);
}

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
{
	if (size == 0) return;
	m_size = size;

	const Context& ctx = Context::get_context();

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateBuffer(ctx.device(), &bufferCreateInfo, nullptr, &m_buf);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(ctx.device(), m_buf, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(ctx.physicalDevice(), &memProperties);

	uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
	for (uint32_t k = 0; k < memProperties.memoryTypeCount; k++)
	{
		if ((memRequirements.memoryTypeBits & (1 << k)) == 0) continue;
		if ((flags & memProperties.memoryTypes[k].propertyFlags) == flags)
		{
			memoryTypeIndex = k;
			break;
		}
	}

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;

	if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)!=0)
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;

	vkAllocateMemory(ctx.device(), &memoryAllocateInfo, nullptr, &m_mem);
	vkBindBufferMemory(ctx.device(), m_buf, m_mem, 0);
}

Buffer::~Buffer()
{
	const Context& ctx = Context::get_context();
	vkDestroyBuffer(ctx.device(), m_buf, nullptr);
	vkFreeMemory(ctx.device(), m_mem, nullptr);
}

UploadBuffer::UploadBuffer(VkDeviceSize size) :
	Buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {	}

UploadBuffer::~UploadBuffer() {}

void UploadBuffer::upload(const void* hdata)
{
	if (m_size == 0) return;

	const Context& ctx = Context::get_context();

	void* data;
	vkMapMemory(ctx.device(), m_mem, 0, m_size, 0, &data);
	memcpy(data, hdata, m_size);
	vkUnmapMemory(ctx.device(), m_mem);
}

void UploadBuffer::zero()
{
	if (m_size == 0) return;

	const Context& ctx = Context::get_context();

	void* data;
	vkMapMemory(ctx.device(), m_mem, 0, m_size, 0, &data);
	memset(data, 0, m_size);
	vkUnmapMemory(ctx.device(), m_mem);
}

DownloadBuffer::DownloadBuffer(VkDeviceSize size) :
	Buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) { }

DownloadBuffer::~DownloadBuffer() {}

void DownloadBuffer::download(void* hdata) const
{
	if (m_size == 0) return;

	const Context& ctx = Context::get_context();

	void* data;
	vkMapMemory(ctx.device(), m_mem, 0, m_size, 0, &data);
	memcpy(hdata, data, m_size);
	vkUnmapMemory(ctx.device(), m_mem);
}

DeviceBuffer::DeviceBuffer(VkDeviceSize size, VkBufferUsageFlags usage) :
	Buffer(size, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {}

DeviceBuffer::~DeviceBuffer() {}

void DeviceBuffer::upload(const void* hdata)
{
	UploadBuffer staging_buf(m_size);
	staging_buf.upload(hdata);

	{
		NTimeCommandBuffer cmdBuf;

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = m_size;
		vkCmdCopyBuffer(cmdBuf.buf(), staging_buf.buf(), m_buf, 1, &copyRegion);
	}
}

void DeviceBuffer::zero()
{
	UploadBuffer staging_buf(m_size);
	staging_buf.zero();

	{
		NTimeCommandBuffer cmdBuf;

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = m_size;
		vkCmdCopyBuffer(cmdBuf.buf(), staging_buf.buf(), m_buf, 1, &copyRegion);
	}
}

void DeviceBuffer::download(void* hdata, VkDeviceSize begin, VkDeviceSize end) const
{
	if (end > m_size) end = m_size;
	if (end <= begin) return;

	DownloadBuffer staging_buf(end - begin);

	{
		NTimeCommandBuffer cmdBuf;

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = begin;
		copyRegion.dstOffset = 0;
		copyRegion.size = end - begin;
		vkCmdCopyBuffer(cmdBuf.buf(), m_buf, staging_buf.buf(), 1, &copyRegion);
	}

	staging_buf.download(hdata);
}

AS::AS() {}

AS::~AS()
{
	const Context& ctx = Context::get_context();
	vkDestroyAccelerationStructureKHR(ctx.device(), m_structure, nullptr);
}

BaseLevelAS::BaseLevelAS(uint32_t geometryCount, const VkAccelerationStructureCreateGeometryTypeInfoKHR* geoTypeInfo,  const VkAccelerationStructureGeometryKHR* pGeometries, const VkAccelerationStructureBuildOffsetInfoKHR* offsets)
{
	const Context& ctx = Context::get_context();

	VkAccelerationStructureCreateInfoKHR acceleration_create_info={};
	acceleration_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	acceleration_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	acceleration_create_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	acceleration_create_info.maxGeometryCount = geometryCount;
	acceleration_create_info.pGeometryInfos = geoTypeInfo;
	vkCreateAccelerationStructureKHR(ctx.device(), &acceleration_create_info, nullptr, &m_structure);	

	VkDeviceSize scratchSizeInBytes = 0;
	VkDeviceSize resultSizeInBytes = 0;
	{		

		VkAccelerationStructureMemoryRequirementsInfoKHR  memoryRequirementsInfo = {};
		memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		memoryRequirementsInfo.accelerationStructure = m_structure;

		VkMemoryRequirements2 memoryRequirements={};
		memoryRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
		vkGetAccelerationStructureMemoryRequirementsKHR(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		resultSizeInBytes = memoryRequirements.memoryRequirements.size;
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
		vkGetAccelerationStructureMemoryRequirementsKHR(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		scratchSizeInBytes = memoryRequirements.memoryRequirements.size;
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_KHR;
		vkGetAccelerationStructureMemoryRequirementsKHR(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		if (memoryRequirements.memoryRequirements.size > scratchSizeInBytes) scratchSizeInBytes = memoryRequirements.memoryRequirements.size;
	}

	m_scratchBuffer = new DeviceBuffer(scratchSizeInBytes, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
	m_resultBuffer = new DeviceBuffer(resultSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);

	VkBindAccelerationStructureMemoryInfoKHR bind_acceleration_memory_info={};
	bind_acceleration_memory_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
	bind_acceleration_memory_info.accelerationStructure = m_structure;
	bind_acceleration_memory_info.memory = m_resultBuffer->memory();
	vkBindAccelerationStructureMemoryKHR(ctx.device(), 1, &bind_acceleration_memory_info);

	VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info={};
	acceleration_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	acceleration_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	acceleration_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	acceleration_build_geometry_info.update = VK_FALSE;
	acceleration_build_geometry_info.dstAccelerationStructure = m_structure;
	acceleration_build_geometry_info.geometryArrayOfPointers = VK_FALSE;
	acceleration_build_geometry_info.geometryCount = geometryCount;
	acceleration_build_geometry_info.ppGeometries = &pGeometries;
	acceleration_build_geometry_info.scratchData.deviceAddress = m_scratchBuffer->get_device_address();

	{
		NTimeCommandBuffer cmdBuf;
		vkCmdBuildAccelerationStructureKHR(cmdBuf.buf(), 1, &acceleration_build_geometry_info, &offsets);
	}

}

BaseLevelAS::~BaseLevelAS()
{
	delete m_resultBuffer;
	delete m_scratchBuffer;
}


TopLevelAS::TopLevelAS(size_t num_hitgroups, size_t* num_instances, const VkAccelerationStructureKHR** pblases, const glm::mat4x4** ptransforms)
{
	const Context& ctx = Context::get_context();

	size_t total = 0;
	for (int i = 0; i < num_hitgroups; i++)
		total += num_instances[i];

	VkAccelerationStructureCreateGeometryTypeInfoKHR acceleration_create_geometry_info={};
	acceleration_create_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR;
	acceleration_create_geometry_info.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	acceleration_create_geometry_info.maxPrimitiveCount = (unsigned)total;

	VkAccelerationStructureCreateInfoKHR acceleration_create_info={};
	acceleration_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	acceleration_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	acceleration_create_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	acceleration_create_info.maxGeometryCount = 1;
	acceleration_create_info.pGeometryInfos = &acceleration_create_geometry_info;
	vkCreateAccelerationStructureKHR(ctx.device(), &acceleration_create_info, nullptr, &m_structure);
	
	VkDeviceSize scratchSizeInBytes, resultSizeInBytes, instanceDescsSizeInBytes;

	{
		VkAccelerationStructureMemoryRequirementsInfoKHR  memoryRequirementsInfo = {};
		memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR;
		memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		memoryRequirementsInfo.accelerationStructure = m_structure;
		
		VkMemoryRequirements2 memoryRequirements = {};
		memoryRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
		vkGetAccelerationStructureMemoryRequirementsKHR(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		resultSizeInBytes = memoryRequirements.memoryRequirements.size;

		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
		vkGetAccelerationStructureMemoryRequirementsKHR(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		scratchSizeInBytes = memoryRequirements.memoryRequirements.size;

		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_KHR;
		vkGetAccelerationStructureMemoryRequirementsKHR(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		if (memoryRequirements.memoryRequirements.size > scratchSizeInBytes) scratchSizeInBytes = memoryRequirements.memoryRequirements.size;

		instanceDescsSizeInBytes = total * sizeof(VkAccelerationStructureInstanceKHR);
	}

	m_scratchBuffer = new DeviceBuffer(scratchSizeInBytes, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
	m_resultBuffer = new DeviceBuffer(resultSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);
	m_instancesBuffer = new DeviceBuffer(instanceDescsSizeInBytes, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR);

	VkBindAccelerationStructureMemoryInfoKHR bind_acceleration_memory_info = {};
	bind_acceleration_memory_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR;
	bind_acceleration_memory_info.accelerationStructure = m_structure;
	bind_acceleration_memory_info.memory = m_resultBuffer->memory();
	vkBindAccelerationStructureMemoryKHR(ctx.device(), 1, &bind_acceleration_memory_info);


	std::vector<VkAccelerationStructureInstanceKHR> geometryInstances(total);
	unsigned k = 0;
	for (int i = 0; i < num_hitgroups; i++)
	{
		for (int j = 0; j < num_instances[i]; j++, k++)
		{
			VkAccelerationStructureInstanceKHR& gInst = geometryInstances[k];

			glm::mat4x4 trans = glm::transpose(ptransforms[i][j]);
			memcpy(&gInst.transform, &trans, sizeof(gInst.transform));
			gInst.instanceCustomIndex = j;
			gInst.mask = 0xFF;
			gInst.instanceShaderBindingTableRecordOffset = i;
			gInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;

			VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info{};
			acceleration_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			acceleration_device_address_info.accelerationStructure = pblases[i][j];

			uint64_t accelerationStructureHandle = vkGetAccelerationStructureDeviceAddressKHR(ctx.device(), &acceleration_device_address_info);
			gInst.accelerationStructureReference = accelerationStructureHandle;
		
		}
	}

	m_instancesBuffer->upload(geometryInstances.data());

	VkAccelerationStructureGeometryKHR geo_instance = {};
	geo_instance.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geo_instance.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	geo_instance.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geo_instance.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geo_instance.geometry.instances.arrayOfPointers = VK_FALSE;
	geo_instance.geometry.instances.data.deviceAddress = m_instancesBuffer->get_device_address();

	VkAccelerationStructureGeometryKHR* p_geo_instance =&geo_instance;

	VkAccelerationStructureBuildGeometryInfoKHR acceleration_build_geometry_info={};
	acceleration_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	acceleration_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	acceleration_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	acceleration_build_geometry_info.update = VK_FALSE;
	acceleration_build_geometry_info.srcAccelerationStructure = VK_NULL_HANDLE;
	acceleration_build_geometry_info.dstAccelerationStructure = m_structure;
	acceleration_build_geometry_info.geometryArrayOfPointers = VK_FALSE;
	acceleration_build_geometry_info.geometryCount = 1;
	acceleration_build_geometry_info.ppGeometries = &p_geo_instance;
	acceleration_build_geometry_info.scratchData.deviceAddress = m_scratchBuffer->get_device_address();

	VkAccelerationStructureBuildOffsetInfoKHR acceleration_build_offset_info={};
	acceleration_build_offset_info.primitiveCount = (unsigned)total;
	acceleration_build_offset_info.primitiveOffset = 0x0;
	acceleration_build_offset_info.firstVertex = 0;
	acceleration_build_offset_info.transformOffset = 0x0;

	VkAccelerationStructureBuildOffsetInfoKHR* p_acceleration_build_offset_info = &acceleration_build_offset_info;
	{
		NTimeCommandBuffer cmdBuf;
		vkCmdBuildAccelerationStructureKHR(cmdBuf.buf(), 1, &acceleration_build_geometry_info, &p_acceleration_build_offset_info);
	}


}


TopLevelAS::~TopLevelAS()
{
	delete m_instancesBuffer;
	delete m_resultBuffer;
	delete m_scratchBuffer;
}

Texture::Texture(int width, int height, int pixel_size, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage)
{
	m_width = width;
	m_height = height;
	m_pixel_size = pixel_size;
	m_format = format;
	if (width == 0 || height == 0) return;

	const Context& ctx = Context::get_context();

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateImage(ctx.device(), &imageInfo, nullptr, &m_image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(ctx.device(), m_image, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(ctx.physicalDevice(), &memProperties);

	uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
	for (uint32_t k = 0; k < memProperties.memoryTypeCount; k++)
	{
		if ((memRequirements.memoryTypeBits & (1 << k)) == 0) continue;
		if ((VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & memProperties.memoryTypes[k].propertyFlags) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			memoryTypeIndex = k;
			break;
		}
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;

	vkAllocateMemory(ctx.device(), &allocInfo, nullptr, &m_mem);
	vkBindImageMemory(ctx.device(), m_image, m_mem, 0);

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = m_image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	vkCreateImageView(ctx.device(), &createInfo, nullptr, &m_view);
}

Texture::~Texture()
{
	if (m_width == 0 || m_height==0) return;
	const Context& ctx = Context::get_context();
	vkDestroyImageView(ctx.device(), m_view, nullptr);
	vkDestroyImage(ctx.device(), m_image, nullptr);
	vkFreeMemory(ctx.device(), m_mem, nullptr);
}

void Texture::uploadTexture(const void* hdata)
{
	if (m_width == 0 || m_height == 0) return;
	UploadBuffer staging_buf(m_width*m_height*m_pixel_size);
	staging_buf.upload(hdata);

	{
		NTimeCommandBuffer cmdBuf;

		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			vkCmdPipelineBarrier(
				cmdBuf.buf(),
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}

		{


			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = {
				(uint32_t)m_width,
				(uint32_t)m_height,
				1
			};

			vkCmdCopyBufferToImage(
				cmdBuf.buf(),
				staging_buf.buf(),
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);
		}

		{

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				cmdBuf.buf(),
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

		}
	}
}


void Texture::downloadTexture(void* hdata) const
{
	if (m_width == 0 || m_height == 0) return;
	DownloadBuffer staging_buf(m_width*m_height*m_pixel_size);

	{
		NTimeCommandBuffer cmdBuf;

		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				cmdBuf.buf(),
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}


		{

			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = {
				(uint32_t)m_width,
				(uint32_t)m_height,
				1
			};

			vkCmdCopyImageToBuffer(
				cmdBuf.buf(),
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				staging_buf.buf(),
				1,
				&region
			);
		}

	}
	staging_buf.download(hdata);

}

Cubemap::Cubemap(int width, int height, int pixel_size, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage)
{
	m_width = width;
	m_height = height;
	m_pixel_size = pixel_size;
	m_format = format;
	if (width == 0 || height == 0) return;

	const Context& ctx = Context::get_context();

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 6;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	vkCreateImage(ctx.device(), &imageInfo, nullptr, &m_image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(ctx.device(), m_image, &memRequirements);

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(ctx.physicalDevice(), &memProperties);

	uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
	for (uint32_t k = 0; k < memProperties.memoryTypeCount; k++)
	{
		if ((memRequirements.memoryTypeBits & (1 << k)) == 0) continue;
		if ((VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT & memProperties.memoryTypes[k].propertyFlags) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			memoryTypeIndex = k;
			break;
		}
	}

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;

	vkAllocateMemory(ctx.device(), &allocInfo, nullptr, &m_mem);
	vkBindImageMemory(ctx.device(), m_image, m_mem, 0);

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = m_image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 6;
	vkCreateImageView(ctx.device(), &createInfo, nullptr, &m_view);
}

Cubemap::~Cubemap()
{
	if (m_width == 0 || m_height == 0) return;
	const Context& ctx = Context::get_context();
	vkDestroyImageView(ctx.device(), m_view, nullptr);
	vkDestroyImage(ctx.device(), m_image, nullptr);
	vkFreeMemory(ctx.device(), m_mem, nullptr);
}

void Cubemap::uploadTexture(const void* hdata)
{
	if (m_width == 0 || m_height == 0) return;
	UploadBuffer staging_buf(m_width*m_height*m_pixel_size*6);
	staging_buf.upload(hdata);

	{
		NTimeCommandBuffer cmdBuf;

		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 6;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			vkCmdPipelineBarrier(
				cmdBuf.buf(),
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}

		{
			VkBufferImageCopy region = {};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 6;
			region.imageExtent = {
				(uint32_t)m_width,
				(uint32_t)m_height,
				1
			};

			vkCmdCopyBufferToImage(
				cmdBuf.buf(),
				staging_buf.buf(),
				m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);
		}

		{

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 6;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				cmdBuf.buf(),
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}
	}
}

Sampler::Sampler()
{
	const Context& ctx = Context::get_context();
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	
	vkCreateSampler(ctx.device(), &samplerInfo, nullptr, &m_sampler);
}

Sampler::~Sampler()
{
	const Context& ctx = Context::get_context();
	vkDestroySampler(ctx.device(), m_sampler, nullptr);
}

