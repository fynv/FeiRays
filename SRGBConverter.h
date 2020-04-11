#pragma once
#include "context.h"

class SRGBConverter
{
public:
	static const SRGBConverter& get_converter();

	void convert(Texture* dst_srgb, DeviceBuffer* src_rgb, float boost = 1.0f) const;

private:
	SRGBConverter();
	~SRGBConverter();

	VkRenderPass m_renderPass;
	VkShaderModule m_vertShaderModule;
	VkShaderModule m_fragShaderModule;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;

	DeviceBuffer* m_ubo;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_descriptorSet;


};
