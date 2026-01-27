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

class HS_API RHITexture : public RHIHandle
{
public:
    ~RHITexture() override;

    const TextureInfo info;

protected:
    RHITexture(const char* name, const TextureInfo& info);
};

class HS_API RHISampler : public RHIHandle
{
public:
    ~RHISampler() override;

    const SamplerInfo info;

protected:
    RHISampler(const char* name, const SamplerInfo& info);
};

class HS_API RHIBuffer : public RHIHandle
{
public:
    ~RHIBuffer() override;

    const BufferInfo info;

    void* byte;
    size_t byteSize;

protected:
    RHIBuffer(const char* name, const BufferInfo& info);
};

class HS_API RHIShader : public RHIHandle
{
public:
    ~RHIShader() override;

    const ShaderInfo info;

protected:
    RHIShader(const char* name, const ShaderInfo& info) noexcept;
};

class HS_API RHIResourceLayout : public RHIHandle
{
public:
    ~RHIResourceLayout() override;

    std::vector<ResourceBinding> bindings;

protected:
    RHIResourceLayout(const char* name, ResourceBinding* bindings, size_t bindingCount);
};

class HS_API RHIResourceSet : public RHIHandle
{
public:
    ~RHIResourceSet() override;

    std::vector<RHIResourceLayout*> layouts;

protected:
    RHIResourceSet(const char* name);
};

class HS_API RHIResourceSetPool : public RHIHandle
{
public:
    ~RHIResourceSetPool() override;

protected:
    RHIResourceSetPool(const char* name);
};

HS_NS_END

namespace std
{
    template <>
    struct hash<hs::RHITexture>
    {
        size_t operator()(const hs::RHITexture& key) const
        {
            return std::hash<hs::TextureInfo>{}(key.info);
        }
    };

    template <>
    struct hash<hs::RHITexture*>
    {
        size_t operator()(const hs::RHITexture* key) const
        {
            return std::hash<hs::TextureInfo>{}(key->info);
        }
    };

    template <>
    struct hash<hs::RHISampler>
    {
        size_t operator()(const hs::RHISampler& key) const
        {
            return std::hash<hs::SamplerInfo>{}(key.info);
        }
    };

    template <>
    struct hash<hs::RHISampler*>
    {
        size_t operator()(const hs::RHISampler* key) const
        {
            return std::hash<hs::SamplerInfo>{}(key->info);
        }
    };

    template <>
    struct hash<hs::RHIBuffer>
    {
        size_t operator()(const hs::RHIBuffer& key) const
        {
            return std::hash<hs::BufferInfo>{}(key.info);
        }
    };

    template <>
    struct hash<hs::RHIBuffer*>
    {
        size_t operator()(const hs::RHIBuffer* key) const
        {
            return std::hash<hs::BufferInfo>{}(key->info);
        }
    };
}

#endif /* __HS_RESOURCE_HANDLE_H__ */
