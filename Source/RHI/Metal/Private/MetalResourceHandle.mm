#include "Core/RHI/Metal/ResourceHandleMetal.h"

#include "Core/RHI/Metal/RHIUtilityMetal.h"
#include "Core/EngineContext.h"

#import <Metal/Metal.h>

HS_NS_BEGIN

TextureMetal::TextureMetal(const char* name, const TextureInfo& info)
    : Texture(name, info)
{
}

TextureMetal::~TextureMetal()
{
}

SamplerMetal::SamplerMetal(const char* name, const SamplerInfo& info)
    : Sampler(name, info)
{}

SamplerMetal::~SamplerMetal()
{}

BufferMetal::BufferMetal(const char* name, const BufferInfo& info)
    : Buffer(name, info)
{
}

BufferMetal::~BufferMetal()
{
}

ShaderMetal::ShaderMetal(const char* name, const ShaderInfo& info)
    : Shader(name, info)
{
}

ShaderMetal::~ShaderMetal()
{
}

ResourceLayoutMetal::ResourceLayoutMetal(const char* name, ResourceBinding* bindings, size_t bindingCount)
    : ResourceLayout(name, bindings, bindingCount)
{
}

ResourceLayoutMetal::~ResourceLayoutMetal()
{
}

ResourceSetMetal::ResourceSetMetal(const char* name)
    : ResourceSet(name)
{
}

ResourceSetMetal::~ResourceSetMetal()
{
}

ResourceSetPoolMetal::ResourceSetPoolMetal(const char* name)
    : ResourceSetPool(name)
{
}

ResourceSetPoolMetal::~ResourceSetPoolMetal()
{
}

HS_NS_END
