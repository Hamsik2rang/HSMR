//
//  ResourceHandleVulkan.h
//  Engine
//
//  Created by Yongsik Im on 4/27/25.
//
#ifndef __HS_RESOURCE_HANDLE_VULKAN_H__
#define __HS_RESOURCE_HANDLE_VULKAN_H__

#include "Precompile.h"

#include "Engine/RHI/ResourceHandle.h"
#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"


HS_NS_BEGIN

struct TextureVulkan : public Texture
{
	TextureVulkan(const char* name, const TextureInfo& info) noexcept : Texture(name, info) {}
	~TextureVulkan() final = default;

	VkImage handle = VK_NULL_HANDLE;
	VkImageView imageViewVk = VK_NULL_HANDLE;
	VkDeviceMemory memoryVk = VK_NULL_HANDLE;
	VkImageLayout layoutVk = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct SamplerVulkan : public Sampler
{
	SamplerVulkan(const char* name,  const SamplerInfo& info) noexcept : Sampler(name, info) {}
	~SamplerVulkan() final = default;

	VkSampler handle = VK_NULL_HANDLE;
};

struct BufferVulkan : public Buffer
{
	BufferVulkan(const char* name, const BufferInfo& info) noexcept : Buffer(name, info) {}
	~BufferVulkan() final = default;

	void Map();
	void Unmap();
	
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct ShaderVulkan : public Shader
{
	ShaderVulkan(const char* name, const ShaderInfo& info) noexcept : Shader(name, info) {}
	~ShaderVulkan() final = default;

	VkShaderModule handle;
	VkPipelineShaderStageCreateInfo stageInfo = {};
};

struct ResourceLayoutVulkan : public ResourceLayout
{
	ResourceLayoutVulkan(const char* name) noexcept : ResourceLayout(name) {}
	~ResourceLayoutVulkan() final = default;

	VkDescriptorSetLayout handle = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayoutBinding> bindingVks;
};

struct ResourceSetVulkan : public ResourceSet
{
	ResourceSetVulkan(const char* name) noexcept : ResourceSet(name) {}
	~ResourceSetVulkan() final = default;

	VkDescriptorSet handle = VK_NULL_HANDLE;
	ResourceLayoutVulkan* layoutVK = nullptr;
};

struct ResourceSetPoolVulkan : public ResourceSetPool
{
	ResourceSetPoolVulkan(const char* name) noexcept : ResourceSetPool(name) {}
	~ResourceSetPoolVulkan() final = default;

	VkDescriptorPool handle = VK_NULL_HANDLE;
};

HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_VULKAN_H__ */