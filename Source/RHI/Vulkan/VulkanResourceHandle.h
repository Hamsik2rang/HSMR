//
//  ResourceHandleVulkan.h
//  Engine
//
//  Created by Yongsik Im on 4/27/25.
//
#ifndef __HS_RESOURCE_HANDLE_VULKAN_H__
#define __HS_RESOURCE_HANDLE_VULKAN_H__

#include "Precompile.h"

#include "RHI/ResourceHandle.h"
#include "RHI/Vulkan/VulkanUtility.h"


HS_NS_BEGIN

struct HS_API TextureVulkan : public RHITexture
{
	TextureVulkan(const char* name, const TextureInfo& info) noexcept : RHITexture(name, info) {}
	~TextureVulkan() final = default;

	VkImage handle = VK_NULL_HANDLE;
	VkImageView imageViewVk = VK_NULL_HANDLE;
	VkDeviceMemory memoryVk = VK_NULL_HANDLE;
	VkImageLayout layoutVk = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct HS_API SamplerVulkan : public RHISampler
{
	SamplerVulkan(const char* name,  const SamplerInfo& info) noexcept : RHISampler(name, info) {}
	~SamplerVulkan() final = default;

	VkSampler handle = VK_NULL_HANDLE;
};

struct HS_API BufferVulkan : public RHIBuffer
{
	BufferVulkan(const char* name, const BufferInfo& info) noexcept : RHIBuffer(name, info) {}
	~BufferVulkan() final = default;

	void Map();
	void Unmap();
	
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct HS_API ShaderVulkan : public RHIShader
{
	ShaderVulkan(const char* name, const ShaderInfo& info) noexcept : RHIShader(name, info) {}
	~ShaderVulkan() final = default;

	VkShaderModule handle;
	VkPipelineShaderStageCreateInfo stageInfo = {};
};

struct HS_API ResourceLayoutVulkan : public RHIResourceLayout
{
	ResourceLayoutVulkan(const char* name, ResourceBinding* bindings, size_t bindingCount) noexcept : RHIResourceLayout(name, bindings, bindingCount) {}
	~ResourceLayoutVulkan() final = default;

	VkDescriptorSetLayout handle = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayoutBinding> bindingVks;
};

struct HS_API ResourceSetVulkan : public RHIResourceSet
{
	ResourceSetVulkan(const char* name) noexcept : RHIResourceSet(name) {}
	~ResourceSetVulkan() final = default;

	VkDescriptorSet handle = VK_NULL_HANDLE;
	ResourceLayoutVulkan* layoutVK = nullptr;
};

struct HS_API ResourceSetPoolVulkan : public RHIResourceSetPool
{
	ResourceSetPoolVulkan(const char* name) noexcept : RHIResourceSetPool(name) {}
	~ResourceSetPoolVulkan() final = default;

	VkDescriptorPool handle = VK_NULL_HANDLE;
};

HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_VULKAN_H__ */