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
    TextureMetal(const TextureInfo& info);
    ~TextureMetal() override;

    id<MTLTexture> handle;
};

struct SamplerMetal : public Sampler
{
    //    SamplerMetal(const SamplerInfo& info);
    SamplerMetal(const SamplerInfo& info);
    ~SamplerMetal() override;

    id<MTLSamplerState> handle;
};

struct BufferMetal : public Buffer
{
    //    BufferMetal(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption);
    BufferMetal(const BufferInfo& info);
    ~BufferMetal() override;

    id<MTLBuffer> handle;
};

struct ShaderMetal : public Shader
{
    ShaderMetal(const ShaderInfo& info, const char* byteCode, size_t byteCodeSize);
    ~ShaderMetal() override;

    id<MTLLibrary> library;
    id<MTLFunction> handle;
};

struct ResourceLayoutMetal : public ResourceLayout
{
    ResourceLayoutMetal();
    ~ResourceLayoutMetal() override;
};

struct ResourceSetMetal : public ResourceSet
{
    ResourceSetMetal();
    ~ResourceSetMetal() override;
};

struct ResourceSetPoolMetal : public ResourceSetPool
{
    ResourceSetPoolMetal();
    ~ResourceSetPoolMetal() override;
};

HS_NS_END

#endif
