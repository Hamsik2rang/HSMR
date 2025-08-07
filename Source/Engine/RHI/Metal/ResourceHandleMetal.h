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
    TextureMetal(const char* name, const TextureInfo& info);
    ~TextureMetal() override;

    id<MTLTexture> handle;
};

struct SamplerMetal : public Sampler
{
    //    SamplerMetal(const SamplerInfo& info);
    SamplerMetal(const char* name, const SamplerInfo& info);
    ~SamplerMetal() override;

    id<MTLSamplerState> handle;
};

struct BufferMetal : public Buffer
{
    //    BufferMetal(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption);
    BufferMetal(const char* name, const BufferInfo& info);
    ~BufferMetal() override;

    id<MTLBuffer> handle;
};

struct ShaderMetal : public Shader
{
    ShaderMetal(const char* name, const ShaderInfo& info);
    ~ShaderMetal() override;

    id<MTLLibrary> library;
    id<MTLFunction> handle;
};

struct ResourceLayoutMetal : public ResourceLayout
{
    ResourceLayoutMetal(const char* name, ResourceBinding* bindings, size_t bindingCount);
    ~ResourceLayoutMetal() override;
};

struct ResourceSetMetal : public ResourceSet
{
    ResourceSetMetal(const char* name);
    ~ResourceSetMetal() override;
};

struct ResourceSetPoolMetal : public ResourceSetPool
{
    ResourceSetPoolMetal(const char* name);
    ~ResourceSetPoolMetal() override;
};

HS_NS_END

#endif
