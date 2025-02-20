#include "Engine/RHI/ResourceHandle.h"

HS_NS_BEGIN

Texture::Texture(const TextureInfo& info)
    : RHIHandle(EType::TEXTURE)
    , info(info)
{
    size_t size = info.extent.width * info.extent.height * info.extent.depth;
}

Texture::~Texture()
{
    if (nullptr != data)
    {
        delete data;
        data = nullptr;
    }
}

Sampler::Sampler(const SamplerInfo& info)
    : RHIHandle(EType::SAMPLER)
    , info(info)
{
}

Sampler::~Sampler()
{
}

Shader::Shader(const char* byteCode, size_t byteCodeSize, const ShaderInfo& info)
    : RHIHandle(EType::SHADER)
    , byteCode(byteCode)
    , byteCodeSize(byteCodeSize)
    , info(info)
{
}

Shader::~Shader()
{
    if (nullptr != byteCode)
    {
        delete[] byteCode;
        byteCode = nullptr;
    }
}

Buffer::Buffer(const BufferInfo& info)
    : RHIHandle(EType::BUFFER)
    , info(info)
{}

Buffer::~Buffer()
{}

ResourceLayout::ResourceLayout()
    : RHIHandle(EType::RESOURCE_LAYOUT)
{}

ResourceLayout::~ResourceLayout()
{}

ResourceSet::ResourceSet()
    : RHIHandle(EType::RESOURCE_SET)
{}

ResourceSet::~ResourceSet()
{}

ResourceSetPool::ResourceSetPool()
    : RHIHandle(EType::RESOURCE_SET_POOL)
{}

ResourceSetPool::~ResourceSetPool()
{}

HS_NS_END
