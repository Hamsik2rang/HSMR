#include "RHI/ResourceHandle.h"

HS_NS_BEGIN

RHITexture::RHITexture(const char* name, const TextureInfo& info)
    : RHIHandle(EType::TEXTURE, name)
    , info(info)
{
    size_t size = info.extent.width * info.extent.height * info.extent.depth;
}

RHITexture::~RHITexture()
{
}

RHISampler::RHISampler(const char* name, const SamplerInfo& info)
    : RHIHandle(EType::SAMPLER, name)
    , info(info)
{
}

RHISampler::~RHISampler()
{
}

RHIShader::RHIShader(const char* name, const ShaderInfo& info) noexcept
    : RHIHandle(EType::SHADER, name)
    , info(info)
{
}

RHIShader::~RHIShader()
{
}

RHIBuffer::RHIBuffer(const char* name, const BufferInfo& info)
    : RHIHandle(EType::BUFFER, name)
    , info(info)
{}

RHIBuffer::~RHIBuffer()
{}

RHIResourceLayout::RHIResourceLayout(const char* name, ResourceBinding* bindings, size_t bindingCount)
    : RHIHandle(EType::RESOURCE_LAYOUT, name)
    , bindings(bindingCount)
{
    HS_ASSERT(bindingCount >= 0, "BindingCount out of range");
    for (size_t i = 0; i < bindingCount; i++)
    {
        this->bindings.push_back(bindings[i]);
    }
}
RHIResourceLayout::~RHIResourceLayout()
{
}

RHIResourceSet::RHIResourceSet(const char* name)
    : RHIHandle(EType::RESOURCE_SET, name)
{}

RHIResourceSet::~RHIResourceSet()
{}

RHIResourceSetPool::RHIResourceSetPool(const char* name)
    : RHIHandle(EType::RESOURCE_SET_POOL, name)
{}

RHIResourceSetPool::~RHIResourceSetPool()
{}
HS_NS_END
