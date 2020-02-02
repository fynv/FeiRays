#include "context.h"

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
			VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
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
		createInfo.enabledExtensionCount = 3;
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
	{
		m_bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_ADDRESS_FEATURES_EXT;
		m_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		m_features2.pNext = &m_bufferDeviceAddressFeatures;
		m_features2.features = {};
		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_features2);
	}

	m_raytracingProperties = {};
	{
		m_raytracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
		VkPhysicalDeviceProperties2 props;
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props.pNext = &m_raytracingProperties;
		props.properties = {};
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
			VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
		#ifdef _WIN64
			VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
		#else
			VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
		#endif
			VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,			
			VK_NV_RAY_TRACING_EXTENSION_NAME,
			
		};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.enabledExtensionCount = 5;
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
	vkDestroyCommandPool(m_device, m_commandPool_graphics, nullptr);
	vkDestroyDevice(m_device, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

OneTimeCommandBuffer::OneTimeCommandBuffer()
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
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(m_buf, &beginInfo);
}

OneTimeCommandBuffer::~OneTimeCommandBuffer()
{
	const Context& ctx = Context::get_context();

	vkEndCommandBuffer(m_buf);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_buf;
	vkQueueSubmit(ctx.queue(), 1, &submitInfo, 0);
	vkQueueWaitIdle(ctx.queue());

	vkFreeCommandBuffers(ctx.device(), ctx.commandPool(), 1, &m_buf);
}

uint64_t Buffer::get_device_address() const
{
	if (m_size == 0) return 0;
	const Context& ctx = Context::get_context();
	VkBufferDeviceAddressInfoEXT bufAdrInfo = {};
	bufAdrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT;
	bufAdrInfo.buffer = m_buf;
	return vkGetBufferDeviceAddressEXT(ctx.device(), &bufAdrInfo);
}

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, bool ext_mem)
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

	VkExportMemoryAllocateInfoKHR vulkanExportMemoryAllocateInfoKHR = {};
	vulkanExportMemoryAllocateInfoKHR.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
#ifdef _WIN64
	vulkanExportMemoryAllocateInfoKHR.pNext = NULL;
	vulkanExportMemoryAllocateInfoKHR.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;
#else
	vulkanExportMemoryAllocateInfoKHR.pNext = NULL;
	vulkanExportMemoryAllocateInfoKHR.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

	if (ext_mem)
		memoryAllocateInfo.pNext = &vulkanExportMemoryAllocateInfoKHR;

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
	Buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false) {	}

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
	Buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false) { }

DownloadBuffer::~DownloadBuffer() {}

void DownloadBuffer::download(void* hdata)
{
	if (m_size == 0) return;

	const Context& ctx = Context::get_context();

	void* data;
	vkMapMemory(ctx.device(), m_mem, 0, m_size, 0, &data);
	memcpy(hdata, data, m_size);
	vkUnmapMemory(ctx.device(), m_mem);
}

DeviceBuffer::DeviceBuffer(VkDeviceSize size, VkBufferUsageFlags usage, bool ext_mem) :
	Buffer(size, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ext_mem) {}

DeviceBuffer::~DeviceBuffer() {}

void DeviceBuffer::upload(const void* hdata)
{
	UploadBuffer staging_buf(m_size);
	staging_buf.upload(hdata);

	{
		OneTimeCommandBuffer cmdBuf;

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
		OneTimeCommandBuffer cmdBuf;

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = m_size;
		vkCmdCopyBuffer(cmdBuf.buf(), staging_buf.buf(), m_buf, 1, &copyRegion);
	}
}

void DeviceBuffer::download(void* hdata, VkDeviceSize begin, VkDeviceSize end)
{
	if (end > m_size) end = m_size;
	if (end <= begin) return;

	DownloadBuffer staging_buf(end - begin);

	{
		OneTimeCommandBuffer cmdBuf;

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
	vkDestroyAccelerationStructureNV(ctx.device(), m_structure, nullptr);
}

BaseLevelAS::BaseLevelAS(uint32_t geometryCount, const VkGeometryNV* pGeometries)
{
	const Context& ctx = Context::get_context();

	VkAccelerationStructureInfoNV accelerationStructureInfo = {};
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	accelerationStructureInfo.geometryCount = geometryCount;
	accelerationStructureInfo.pGeometries = pGeometries;

	VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo = {};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCreateInfo.info = accelerationStructureInfo;

	vkCreateAccelerationStructureNV(ctx.device(), &accelerationStructureCreateInfo, nullptr, &m_structure);

	VkDeviceSize scratchSizeInBytes = 0;
	VkDeviceSize resultSizeInBytes = 0;

	{
		VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = {};
		memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
		memoryRequirementsInfo.accelerationStructure = m_structure;

		VkMemoryRequirements2 memoryRequirements;
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		resultSizeInBytes = memoryRequirements.memoryRequirements.size;
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		scratchSizeInBytes = memoryRequirements.memoryRequirements.size;
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		if (memoryRequirements.memoryRequirements.size > scratchSizeInBytes) scratchSizeInBytes = memoryRequirements.memoryRequirements.size;
	}

	m_scratchBuffer = new DeviceBuffer(scratchSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_resultBuffer = new DeviceBuffer(resultSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

	{
		OneTimeCommandBuffer cmdBuf;

		VkBindAccelerationStructureMemoryInfoNV bindInfo = {};
		bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
		bindInfo.accelerationStructure = m_structure;
		bindInfo.memory = m_resultBuffer->memory();

		vkBindAccelerationStructureMemoryNV(ctx.device(), 1, &bindInfo);

		vkCmdBuildAccelerationStructureNV(cmdBuf.buf(), &accelerationStructureInfo, VK_NULL_HANDLE, 0, VK_FALSE,
			m_structure, VK_NULL_HANDLE, m_scratchBuffer->buf(), 0);
	}

}

BaseLevelAS::~BaseLevelAS()
{
	delete m_resultBuffer;
	delete m_scratchBuffer;
}

struct VkGeometryInstance
{
	/// Transform matrix, containing only the top 3 rows
	float transform[12];
	/// Instance index
	uint32_t instanceId : 24;
	/// Visibility mask
	uint32_t mask : 8;
	/// Index of the hit group which will be invoked when a ray hits the instance
	uint32_t instanceOffset : 24;
	/// Instance flags, such as culling
	uint32_t flags : 8;
	/// Opaque handle of the bottom-level acceleration structure
	uint64_t accelerationStructureHandle;
};

TopLevelAS::TopLevelAS(size_t num_hitgroups, size_t* num_instances, const VkAccelerationStructureNV** pblases, const glm::mat4x4** ptransforms)
{
	const Context& ctx = Context::get_context();

	size_t total = 0;
	for (int i = 0; i < num_hitgroups; i++)
		total += num_instances[i];

	VkAccelerationStructureInfoNV accelerationStructureInfo = {};
	accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	accelerationStructureInfo.instanceCount = (unsigned)total;

	VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo = {};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	accelerationStructureCreateInfo.info = accelerationStructureInfo;

	vkCreateAccelerationStructureNV(ctx.device(), &accelerationStructureCreateInfo, nullptr, &m_structure);

	VkDeviceSize scratchSizeInBytes, resultSizeInBytes, instanceDescsSizeInBytes;

	{
		VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo = {};
		memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
		memoryRequirementsInfo.accelerationStructure = m_structure;

		VkMemoryRequirements2 memoryRequirements;

		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		resultSizeInBytes = memoryRequirements.memoryRequirements.size;

		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		scratchSizeInBytes = memoryRequirements.memoryRequirements.size;

		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV;
		vkGetAccelerationStructureMemoryRequirementsNV(ctx.device(), &memoryRequirementsInfo, &memoryRequirements);
		if (memoryRequirements.memoryRequirements.size > scratchSizeInBytes) scratchSizeInBytes = memoryRequirements.memoryRequirements.size;

		instanceDescsSizeInBytes = total * sizeof(VkGeometryInstance);
	}

	m_scratchBuffer = new DeviceBuffer(scratchSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_resultBuffer = new DeviceBuffer(resultSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
	m_instancesBuffer = new DeviceBuffer(instanceDescsSizeInBytes, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);

	std::vector<VkGeometryInstance> geometryInstances(total);
	unsigned k = 0;
	for (int i = 0; i < num_hitgroups; i++)
	{
		for (int j = 0; j < num_instances[i]; j++, k++)
		{
			uint64_t accelerationStructureHandle = 0;
			vkGetAccelerationStructureHandleNV(ctx.device(), pblases[i][j], sizeof(uint64_t), &accelerationStructureHandle);

			VkGeometryInstance& gInst = geometryInstances[k];
			glm::mat4x4 trans = glm::transpose(ptransforms[i][j]);
			memcpy(gInst.transform, &trans, sizeof(gInst.transform));
			gInst.instanceId = j;
			gInst.mask = 0xff;
			gInst.instanceOffset = i;
			gInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
			gInst.accelerationStructureHandle = accelerationStructureHandle;
		}
	}

	VkDeviceSize instancesBufferSize = total * sizeof(VkGeometryInstance);
	m_instancesBuffer->upload(geometryInstances.data());

	{
		OneTimeCommandBuffer cmdBuf;

		VkBindAccelerationStructureMemoryInfoNV bindInfo = {};
		bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
		bindInfo.accelerationStructure = m_structure;
		bindInfo.memory = m_resultBuffer->memory();

		vkBindAccelerationStructureMemoryNV(ctx.device(), 1, &bindInfo);

		vkCmdBuildAccelerationStructureNV(cmdBuf.buf(), &accelerationStructureInfo, m_instancesBuffer->buf(), 0, VK_FALSE,
			m_structure, VK_NULL_HANDLE, m_scratchBuffer->buf(), 0);
	}


}


TopLevelAS::~TopLevelAS()
{
	delete m_instancesBuffer;
	delete m_resultBuffer;
	delete m_scratchBuffer;
}
