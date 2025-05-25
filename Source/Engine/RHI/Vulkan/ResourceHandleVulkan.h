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

HS_NS_BEGIN

struct TextureVulkan : public Texture
{
	TextureVulkan(const TextureInfo& info) : Texture(info) {}
	~TextureVulkan() override;
};

struct SamplerVulkan : public Sampler
{
	SamplerVulkan(const SamplerInfo& info) : Sampler(info) {}
	~SamplerVulkan() override;
};

struct BufferVulkan : public Buffer
{
	BufferVulkan(const BufferInfo& info) : Buffer(info) {}
	~BufferVulkan() override;
};

struct ShaderVulkan : public Shader
{
	ShaderVulkan(const ShaderInfo& info) : Shader(info) {}
	~ShaderVulkan() override;
};

struct ResourceLayoutVulkan : public ResourceLayout
{
	~ResourceLayoutVulkan() {}
};

struct ResourceSetVulkan : public ResourceSet
{

};

struct ResourceSetPoolVulkan : public ResourceSetPool
{

};


HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_VULKAN_H__ */