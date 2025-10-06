#include "RHI/Metal/MetalResourceHandle.h"

#include "RHI/Metal/MetalUtility.h"

#import <Metal/Metal.h>

HS_NS_BEGIN

MetalTexture::MetalTexture(const char* name, const TextureInfo& info)
    : RHITexture(name, info)
{
}

MetalTexture::~MetalTexture()
{
}

MetalSampler::MetalSampler(const char* name, const SamplerInfo& info)
    : RHISampler(name, info)
{}

MetalSampler::~MetalSampler()
{}

MetalBuffer::MetalBuffer(const char* name, const BufferInfo& info)
    : RHIBuffer(name, info)
{
}

MetalBuffer::~MetalBuffer()
{
}

MetalShader::MetalShader(const char* name, const ShaderInfo& info)
    : RHIShader(name, info)
{
}

MetalShader::~MetalShader()
{
}

MetalResourceLayout::MetalResourceLayout(const char* name, ResourceBinding* bindings, size_t bindingCount)
    : RHIResourceLayout(name, bindings, bindingCount)
{
}

MetalResourceLayout::~MetalResourceLayout()
{
}

MetalResourceSet::MetalResourceSet(const char* name)
    : RHIResourceSet(name)
{
}

MetalResourceSet::~MetalResourceSet()
{
}

MetalResourceSetPool::MetalResourceSetPool(const char* name)
    : RHIResourceSetPool(name)
{
}

MetalResourceSetPool::~MetalResourceSetPool()
{
}

HS_NS_END
