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
    Texture(const TextureInfo& info);
    ~Texture() override;
   
    void* data;
    uint32 dataSize;
    
    const TextureInfo info;
};

struct Sampler : public RHIHandle
{
    Sampler(const SamplerInfo& info);
    ~Sampler() override;

    const SamplerInfo info;
};

struct Buffer : public RHIHandle
{
    Buffer(const BufferInfo& info);
    ~Buffer() override;

    void*            byte;
    size_t           byteSize;
    
    const BufferInfo info;
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

    std::vector<ResourceBinding> bindings;
};

struct ResourceSet : public RHIHandle
{
    ResourceSet();
    ~ResourceSet() override;

    std::vector<ResourceLayout*> layouts;
};

struct ResourceSetPool : public RHIHandle
{
    ResourceSetPool();
    ~ResourceSetPool() override;
};

HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_H__ */
