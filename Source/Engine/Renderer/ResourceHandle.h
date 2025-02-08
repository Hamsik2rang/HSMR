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

struct Texture
{
    Texture(void* image, const TextureInfo& texInfo, const SamplerInfo& smpInfo);
    ~Texture();
        
    TextureInfo texInfo;
    SamplerInfo smpInfo;
};

struct Buffer
{
    Buffer(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption);
    ~Buffer();
    
    EBufferUsage _usage;
    EBufferMemoryOption _memoryOption;
    void* byte;
    size_t byteSize;
};



HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_H__ */
