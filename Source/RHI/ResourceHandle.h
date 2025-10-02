//
//  ResourceHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/9/25.
//
#ifndef __HS_RESOURCE_HANDLE_H__
#define __HS_RESOURCE_HANDLE_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"

HS_NS_BEGIN

class HS_RHI_API RHITexture : public RHIHandle
{
public:
    ~RHITexture() override;

    const TextureInfo info;

protected:
    RHITexture(const char* name, const TextureInfo& info);
};

class HS_RHI_API RHISampler : public RHIHandle
{
public:
    ~RHISampler() override;

    const SamplerInfo info;

protected:
    RHISampler(const char* name, const SamplerInfo& info);
};

class HS_RHI_API RHIBuffer : public RHIHandle
{
public:
    ~RHIBuffer() override;

    const BufferInfo info;

    void* byte;
    size_t byteSize;

protected:
    RHIBuffer(const char* name, const BufferInfo& info);
};

class HS_RHI_API RHIShader : public RHIHandle
{
public:
    ~RHIShader() override;

    const ShaderInfo info;

protected:
    RHIShader(const char* name, const ShaderInfo& info) noexcept;
};

class HS_RHI_API RHIResourceLayout : public RHIHandle
{
public:
    ~RHIResourceLayout() override;

    std::vector<ResourceBinding> bindings;

protected:
    RHIResourceLayout(const char* name, ResourceBinding* bindings, size_t bindingCount);
};

class HS_RHI_API RHIResourceSet : public RHIHandle
{
public:
    ~RHIResourceSet() override;

    std::vector<RHIResourceLayout*> layouts;

protected:
    RHIResourceSet(const char* name);
};

class HS_RHI_API RHIResourceSetPool : public RHIHandle
{
public:
    ~RHIResourceSetPool() override;

protected:
    RHIResourceSetPool(const char* name);
};

template <>
struct Hasher<RHITexture>
{
    static uint32 Get(const RHITexture& key)
    {
        return Hasher<TextureInfo>::Get(key.info);
    }
};

template <>
struct Hasher<RHITexture*>
{
    static uint32 Get(const RHITexture*& key)
    {
        return Hasher<TextureInfo>::Get(key->info);
    }
};

template <>
struct Hasher<RHISampler>
{
    static uint32 Get(const RHISampler& key)
    {
        return Hasher<SamplerInfo>::Get(key.info);
    }
};

template <>
struct Hasher<RHISampler*>
{
    static uint32 Get(const RHISampler*& key)
    {
        return Hasher<SamplerInfo>::Get(key->info);
    }
};

template <>
struct Hasher<RHIBuffer>
{
    static uint32 Get(const RHIBuffer& key)
    {
        return Hasher<BufferInfo>::Get(key.info);
    }
};

template <>
struct Hasher<RHIBuffer*>
{
    static uint32 Get(const RHIBuffer*& key)
    {
        return Hasher<BufferInfo>::Get(key->info);
    }
};

HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_H__ */
