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
	TextureVulkan(const TextureInfo& info) noexcept : Texture(info) {}
	~TextureVulkan() override = default;

	VkImage handle = VK_NULL_HANDLE;
	VkImageView imageViewVk = VK_NULL_HANDLE;
	VkDeviceMemory memoryVk = VK_NULL_HANDLE;
	VkImageLayout layoutVk = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct SamplerVulkan : public Sampler
{
	SamplerVulkan(const SamplerInfo& info) noexcept : Sampler(info) {}
	~SamplerVulkan() override = default;

	VkSampler handle = VK_NULL_HANDLE;
};

struct BufferVulkan : public Buffer
{
	BufferVulkan(const BufferInfo& info) noexcept : Buffer(info) {}
	~BufferVulkan() override = default;

	void Map();
	void Unmap();
	
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct ShaderVulkan : public Shader
{
	ShaderVulkan(const ShaderInfo& info) noexcept : Shader(info) {}
	~ShaderVulkan() override = default;

	VkShaderModule handle;
	VkPipelineShaderStageCreateInfo stageInfo = {};
};

struct ResourceLayoutVulkan : public ResourceLayout
{
	ResourceLayoutVulkan() noexcept : ResourceLayout() {}
	~ResourceLayoutVulkan() override = default;

	VkDescriptorSetLayout handle = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayoutBinding> bindingVks;
};

struct ResourceSetVulkan : public ResourceSet
{
	~ResourceSetVulkan() override = default;

	VkDescriptorSet handle = VK_NULL_HANDLE;
	ResourceLayoutVulkan* layoutVK = nullptr;
};


HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_VULKAN_H__ */