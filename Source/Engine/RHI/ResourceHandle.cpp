#include "Engine/RHI/ResourceHandle.h"

HS_NS_BEGIN

Texture::Texture()
    : RHIHandle(EType::TEXTURE)
{}

Texture::~Texture()
{}

Sampler::Sampler()
    : RHIHandle(EType::SAMPLER)
{}

Sampler::~Sampler()
{}

Shader::Shader(const char* byteCode, size_t byteCodeSize, const ShaderInfo& info)
    : RHIHandle(EType::SHADER)
    , byteCode(byteCode)
    , byteCodeSize(byteCodeSize)
    , info(info)
{}

Shader::~Shader()
{
    if (nullptr != byteCode)
    {
        delete[] byteCode;
        byteCode = nullptr;
    }
}

Buffer::Buffer()
    : RHIHandle(EType::BUFFER)
{}

Buffer::~Buffer()
{}

ResourceLayout::ResourceLayout()
    : RHIHandle(EType::RESOURCE_LAYOUT)
{}

ResourceLayout::~ResourceLayout()
{}

DescriptorSet::DescriptorSet()
    : RHIHandle(EType::DESCRIPTOR_SET)
{}

DescriptorSet::~DescriptorSet()
{}

DescriptorPool::DescriptorPool()
    : RHIHandle(EType::DESCRIPTOR_POOL)
{}

DescriptorPool::~DescriptorPool()
{}

HS_NS_END
