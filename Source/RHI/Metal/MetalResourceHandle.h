//
//  MetalResourceHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RESOURCE_HANDLE_METAL_H__
#define __HS_RESOURCE_HANDLE_METAL_H__

#include "Precompile.h"

#include "RHI/ResourceHandle.h"
#include "RHI/Metal/MetalUtility.h"

HS_NS_BEGIN

struct MetalTexture : public RHITexture
{
    //    MetalTexture(void* image, const TextureInfo& texInfo);
    MetalTexture(const char* name, const TextureInfo& info);
    ~MetalTexture() override;

    id<MTLTexture> handle;
};

struct MetalSampler : public RHISampler
{
    //    MetalSampler(const SamplerInfo& info);
    MetalSampler(const char* name, const SamplerInfo& info);
    ~MetalSampler() override;

    id<MTLSamplerState> handle;
};

struct MetalBuffer : public RHIBuffer
{
    //    MetalBuffer(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption);
    MetalBuffer(const char* name, const BufferInfo& info);
    ~MetalBuffer() override;

    id<MTLBuffer> handle;
};

struct MetalShader : public RHIShader
{
    MetalShader(const char* name, const ShaderInfo& info);
    ~MetalShader() override;

    id<MTLLibrary> library;
    id<MTLFunction> handle;
};

struct MetalResourceLayout : public RHIResourceLayout
{
    MetalResourceLayout(const char* name, ResourceBinding* bindings, size_t bindingCount);
    ~MetalResourceLayout() override;
};

struct MetalResourceSet : public RHIResourceSet
{
    MetalResourceSet(const char* name);
    ~MetalResourceSet() override;
};

struct MetalResourceSetPool : public RHIResourceSetPool
{
    MetalResourceSetPool(const char* name);
    ~MetalResourceSetPool() override;
};

HS_NS_END

#endif
