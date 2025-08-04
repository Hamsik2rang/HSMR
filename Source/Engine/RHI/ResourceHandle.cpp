#include "Engine/RHI/ResourceHandle.h"

HS_NS_BEGIN

Texture::Texture(const char* name, const TextureInfo& info)
    : RHIHandle(EType::TEXTURE, name)
    , info(info)
{
    size_t size = info.extent.width * info.extent.height * info.extent.depth;
}

Texture::~Texture()
{

}

Sampler::Sampler(const char* name, const SamplerInfo& info)
    : RHIHandle(EType::SAMPLER, name)
    , info(info)
{
}

Sampler::~Sampler()
{
}

Shader::Shader(const char* name, const ShaderInfo& info) noexcept
    : RHIHandle(EType::SHADER, name)
    , info(info)
{
}

Shader::~Shader()
{

}

Buffer::Buffer(const char* name, const BufferInfo& info)
    : RHIHandle(EType::BUFFER, name)
    , info(info)
{}

Buffer::~Buffer()
{}

ResourceLayout::ResourceLayout(const char* name)
    : RHIHandle(EType::RESOURCE_LAYOUT, name)

{

}
ResourceLayout::~ResourceLayout()
{

}

ResourceSet::ResourceSet(const char* name)
    : RHIHandle(EType::RESOURCE_SET, name)
{}

ResourceSet::~ResourceSet()
{}


ResourceSetPool::ResourceSetPool(const char* name)
	: RHIHandle(EType::RESOURCE_SET_POOL, name)
{}

ResourceSetPool::~ResourceSetPool()
{}
HS_NS_END
