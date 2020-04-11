#include <stdio.h>
#include <string>
#include "RNGInitializer.h"
#include "xor_wow_data.hpp"
#include "RNGState_xorwow.h"

const RNGInitializer& RNGInitializer::get_initializer()
{
	static RNGInitializer initializer;
	return initializer;
}


static VkShaderModule _createShaderModule_from_spv(const char* fn)
{
	std::string fullname = "../shaders/";
	fullname += fn;

	const Context& ctx = Context::get_context();

	FILE* fp = fopen(fullname.data(), "rb");
	fseek(fp, 0, SEEK_END);
	size_t bytes = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* buf = new char[bytes];
	fread(buf, 1, bytes, fp);

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = bytes;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buf);
	VkShaderModule shaderModule;
	vkCreateShaderModule(ctx.device(), &createInfo, nullptr, &shaderModule);

	delete[] buf;

	fclose(fp);

	return shaderModule;
}


RNGInitializer::RNGInitializer()
{
	m_seq_mat = new DeviceBuffer(sizeof(xorwow_sequence_matrix), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT);
	m_seq_mat->upload(xorwow_sequence_matrix);

	const Context& ctx = Context::get_context();

	{
		VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2];
		descriptorSetLayoutBindings[0] = {};
		descriptorSetLayoutBindings[0].binding = 0;
		descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorSetLayoutBindings[0].descriptorCount = 1;
		descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		descriptorSetLayoutBindings[1] = {};
		descriptorSetLayoutBindings[1].binding = 1;
		descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetLayoutBindings[1].descriptorCount = 1;
		descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 2;
		layoutInfo.pBindings = descriptorSetLayoutBindings;

		vkCreateDescriptorSetLayout(ctx.device(), &layoutInfo, nullptr, &m_descriptorSetLayout);

	}
	{
		VkShaderModule compModule = _createShaderModule_from_spv("common/rand_init.spv");

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

		vkCreatePipelineLayout(ctx.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

		VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = compModule;
		computeShaderStageInfo.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = computeShaderStageInfo;
		pipelineInfo.layout = m_pipelineLayout;

		vkCreateComputePipelines(ctx.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);

		vkDestroyShaderModule(ctx.device(), compModule, nullptr);
	}

	m_ubo = new DeviceBuffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	{
		VkDescriptorPoolSize poolSizes[2];
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 2;
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 1;
		vkCreateDescriptorPool(ctx.device(), &poolInfo, nullptr, &m_descriptorPool);
	}

	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_descriptorSetLayout;
		vkAllocateDescriptorSets(ctx.device(), &allocInfo, &m_descriptorSet);
	}
}

RNGInitializer::~RNGInitializer()
{
	/*
	delete m_ubo;
	const Context& ctx = Context::get_context();
	vkDestroyPipeline(ctx.device(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(ctx.device(), m_pipelineLayout, nullptr);
	vkDestroyDescriptorPool(ctx.device(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(ctx.device(), m_descriptorSetLayout, nullptr);
	delete m_seq_mat;*/
}

void RNGInitializer::InitRNGs(DeviceBuffer* rng_states) const
{
	const Context& ctx = Context::get_context();

	size_t count = rng_states->size() / sizeof(RNGState);
	UBO ubo_data;
	ubo_data.d_sequence_matrix = m_seq_mat->get_device_address();
	ubo_data.count = (int)count;
	m_ubo->upload(&ubo_data);

	VkDescriptorBufferInfo ssboInfo = {};
	ssboInfo.buffer = rng_states->buf();
	ssboInfo.range = VK_WHOLE_SIZE;

	VkDescriptorBufferInfo uboInfo = {};
	uboInfo.buffer = m_ubo->buf();
	uboInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrites[2];
	descriptorWrites[0] = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = m_descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &ssboInfo;

	descriptorWrites[1] = {};
	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = m_descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = &uboInfo;

	vkUpdateDescriptorSets(ctx.device(), 2, descriptorWrites, 0, nullptr);

	{
		NTimeCommandBuffer cmdBuf;
		vkCmdBindPipeline(cmdBuf.buf(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
		vkCmdBindDescriptorSets(cmdBuf.buf(), VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, 0);
		vkCmdDispatch(cmdBuf.buf(), (unsigned)((count + 127) / 128), 1, 1);
	}

}



