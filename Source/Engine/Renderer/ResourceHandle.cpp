#include "Engine/Renderer/ResourceHandle.h"

HS_NS_BEGIN

Texture::Texture(void* image, const TextureInfo& texInfo, const SamplerInfo& smpInfo)
    : texInfo(texInfo)
    , smpInfo(smpInfo)
{
}

Buffer::Buffer(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption)
:_usage(usage)
, _memoryOption(memoryOption)
{
}

HS_NS_END
