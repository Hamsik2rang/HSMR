#include "RHI/ResourceHandle.h"

HS_NS_BEGIN

RHITexture::RHITexture(const char* name, const TextureInfo& info)
    : RHIHandle(EType::Texture, name)
    , info(info)
{
    size_t size = info.extent.width * info.extent.height * info.extent.depth;
}

RHITexture::~RHITexture()
{
}

RHISampler::RHISampler(const char* name, const SamplerInfo& info)
    : RHIHandle(EType::Sampler, name)
    , info(info)
{
}

RHISampler::~RHISampler()
{
}

RHIShader::RHIShader(const char* name, const ShaderInfo& info) noexcept
    : RHIHandle(EType::Shader, name)
    , info(info)
{
}

RHIShader::~RHIShader()
{
}

RHIBuffer::RHIBuffer(const char* name, const BufferInfo& info)
    : RHIHandle(EType::Buffer, name)
    , info(info)
{}

RHIBuffer::~RHIBuffer()
{}

RHIResourceLayout::RHIResourceLayout(const char* name, ResourceBinding* bindings, size_t bindingCount)
    : RHIHandle(EType::ResourceLayout, name)
    , bindings(bindingCount)
{
    HS_ASSERT(bindingCount >= 0, "BindingCount out of range");
    for (size_t i = 0; i < bindingCount; i++)
    {
        this->bindings[i] = bindings[i];
    }
}
RHIResourceLayout::~RHIResourceLayout()
{
}

RHIResourceSet::RHIResourceSet(const char* name)
    : RHIHandle(EType::ResourceSet, name)
{}

RHIResourceSet::~RHIResourceSet()
{}

RHIResourceSetPool::RHIResourceSetPool(const char* name)
    : RHIHandle(EType::ResourceSetPool, name)
{}

RHIResourceSetPool::~RHIResourceSetPool()
{}
HS_NS_END
