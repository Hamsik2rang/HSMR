//
//  ResourceHandleMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RESOURCE_HANDLE_METAL_H__
#define __HS_RESOURCE_HANDLE_METAL_H__

#include "Precompile.h"

#include "Engine/RHI/ResourceHandle.h"
#include "Engine/RHI/Metal/RHIUtilityMetal.h"

HS_NS_BEGIN

struct TextureMetal : public Texture
{
//    TextureMetal(void* image, const TextureInfo& texInfo);
    TextureMetal();
    ~TextureMetal() override;
};

struct SamplerMetal : public Sampler
{
//    SamplerMetal(const SamplerInfo& info);
    SamplerMetal();
    ~SamplerMetal() override;
};

struct BufferMetal : public Buffer
{
//    BufferMetal(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption);
    BufferMetal();
    ~BufferMetal() override;
};

struct ShaderMetal : public Shader
{
    ShaderMetal(const char* byteCode, size_t byteCodeSize, const ShaderInfo& info);
    ~ShaderMetal() override;
};

struct ResourceLayoutMetal : public ResourceLayout
{
    ResourceLayoutMetal();
    ~ResourceLayoutMetal() override;
};

struct DescriptorSetMetal : public DescriptorSet
{
    DescriptorSetMetal();
    ~DescriptorSetMetal() override;
};

struct DescriptorPoolMetal : public DescriptorPool
{
    DescriptorPoolMetal();
    ~DescriptorPoolMetal() override;
};

HS_NS_END

#endif
