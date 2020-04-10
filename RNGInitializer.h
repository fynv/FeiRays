#pragma once
#include "context.h"

class RNGInitializer
{
public:
	static const RNGInitializer& get_initializer();
	void InitRNGs(DeviceBuffer* rng_states) const;

private:
	RNGInitializer();
	~RNGInitializer();

	struct UBO
	{
		VkDeviceAddress d_sequence_matrix;
		int count;
	};

	DeviceBuffer* m_seq_mat;
	VkDescriptorSetLayout m_descriptorSetLayout;

	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_pipeline;

	DeviceBuffer* m_ubo;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_descriptorSet;

};

