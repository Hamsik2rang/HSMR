//
//  ResourceHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/9/25.
//
#ifndef __HS_RESOURCE_HANDLE_H__
#define __HS_RESOURCE_HANDLE_H__

#include "Precompile.h"

#include "Engine/Renderer/RenderDefinition.h"
#include <simd/simd.h>

HS_NS_BEGIN

class Texture : public RHIHandle
{
public:
    Texture(void* image, const TextureInfo& texInfo, const SamplerInfo& smpInfo);
    Texture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage );
    ~Texture() override;

    TextureInfo texInfo{};
    SamplerInfo smpInfo{};
};

class Buffer : public RHIHandle
{
public:
    Buffer(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption);
    ~Buffer() override;

    EBufferUsage        _usage;
    EBufferMemoryOption _memoryOption;
    void*               byte;
    size_t              byteSize;
};

HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_H__ */
