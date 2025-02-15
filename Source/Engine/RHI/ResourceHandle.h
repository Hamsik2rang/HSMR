//
//  ResourceHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/9/25.
//
#ifndef __HS_RESOURCE_HANDLE_H__
#define __HS_RESOURCE_HANDLE_H__

#include "Precompile.h"

#include "Engine/RHI/RHIDefinition.h"
#include <simd/simd.h>

HS_NS_BEGIN

struct Texture : public RHIHandle
{
    Texture();
    ~Texture() override;

    TextureInfo GetInfo() { return _info; }

    TextureInfo _info;
};

struct Sampler : public RHIHandle
{
    Sampler();
    ~Sampler() override;

    SamplerInfo GetInfo() { return _info; }

    SamplerInfo _info;
};

struct Buffer : public RHIHandle
{
    Buffer();
    //    Buffer(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption);
    ~Buffer() override;

    EBufferUsage        _usage;
    EBufferMemoryOption _memoryOption;
    void*               byte;
    size_t              byteSize;
};

struct Shader : public RHIHandle
{
    Shader(const char* byteCode, size_t byteCodeSize, const ShaderInfo& info);
    ~Shader() override;

    const ShaderInfo info;
    const char*      byteCode;
    const size_t     byteCodeSize;
};

struct ResourceLayout : public RHIHandle
{
    ResourceLayout();
    ~ResourceLayout() override;

    std::vector<ResourceBinding> _bindings;
    uint32                       hash;
};

struct DescriptorSet : public RHIHandle
{
    DescriptorSet();
    ~DescriptorSet() override;

    std::vector<ResourceLayout> _layouts;
};

struct DescriptorPool : public RHIHandle
{
    DescriptorPool();
    ~DescriptorPool() override;
};

HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_H__ */
