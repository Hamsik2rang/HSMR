#include "Engine/RHI/Metal/ResourceHandleMetal.h"

#include "Engine/RHI/Metal/RHIUtilityMetal.h"
#include "Engine/Core/EngineContext.h"

#import <Metal/Metal.h>

HS_NS_BEGIN

// TextureMetal::TextureMetal(void* image, const TextureInfo& texInfo)
//{
//
//
//     MTLTextureDescriptor* texDesc     = [[MTLTextureDescriptor alloc] init];
//     texDesc.width                     = texInfo.extent.width;
//     texDesc.height                    = texInfo.extent.height;
//     texDesc.depth                     = texInfo.extent.depth;
//     texDesc.arrayLength               = texInfo.arrayLength;
//     texDesc.allowGPUOptimizedContents = true;
//     texDesc.usage                     = hs_rhi_to_texture_usage(texInfo.usage);
//     texDesc.sampleCount               = 1;
//     texDesc.pixelFormat               = hs_rhi_to_pixel_format(texInfo.format);
//     texDesc.textureType               = hs_rhi_to_texture_type(texInfo.type);
//
//     id<MTLTexture> texture = [device newTextureWithDescriptor:texDesc];
//
//     handle = hs_rhi_from_texture(texture);
// }
//
// TextureMetal::TextureMetal(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage)
//{
//     id<MTLDevice> device = hs_rhi_to_device(hs_engine_get_rhi_context());
//
//     MTLTextureDescriptor* texDesc     = [[MTLTextureDescriptor alloc] init];
//     texDesc.width                     = width;
//     texDesc.height                    = height;
//     texDesc.depth                     = 1;
//     texDesc.arrayLength               = 1;
//     texDesc.allowGPUOptimizedContents = true;
//     texDesc.usage                     = hs_rhi_to_texture_usage(usage);
//     texDesc.sampleCount               = 1;
//     texDesc.pixelFormat               = hs_rhi_to_pixel_format(format);
//     texDesc.textureType               = hs_rhi_to_texture_type(type);
//
//     id<MTLTexture> texture = [device newTextureWithDescriptor:texDesc];
// }

TextureMetal::TextureMetal()
{
}

TextureMetal::~TextureMetal()
{
}

SamplerMetal::SamplerMetal()
{}

SamplerMetal::~SamplerMetal()
{}

// BufferMetal::BufferMetal(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption)
//     : _usage(usage)
//     , _memoryOption(memoryOption)
//{
//
// }

BufferMetal::BufferMetal()
{
}

BufferMetal::~BufferMetal()
{
}

// ShaderMetal::ShaderMetal(Shader::EStage stage, const char* path, const char* entryName, bool isBuiltIn)
//     : Object(EType::SHADER)
//     , _byteCode(nullptr)
//     , _byteSize(0)
//     , _stage(stage)
//     , _entryName(entryName)
//     , _isBuiltIn(isBuiltIn)
//{
//     loadShader();
// }

// ShaderMetal::ShaderMetal(Shader::EStage stage, const char* byteCode, size_t byteSize, const char* entryName, bool isBuiltIn)
//     : Object(EType::SHADER)
//     , _byteCode(byteCode)
//     , _byteSize(byteSize)
//     , _stage(stage)
//     , _entryName(entryName)
//     , _isBuiltIn(isBuiltIn)
//{
//     loadShader();
// }

ShaderMetal::ShaderMetal(const char* byteCode, size_t byteCodeSize, const ShaderInfo& info)
    : Shader(byteCode, byteCodeSize, info)
{
}

ShaderMetal::~ShaderMetal()
{
}

ResourceLayoutMetal::ResourceLayoutMetal()
{
}

ResourceLayoutMetal::~ResourceLayoutMetal()
{
}

DescriptorSetMetal::DescriptorSetMetal()
{
}

DescriptorSetMetal::~DescriptorSetMetal()
{
}

DescriptorPoolMetal::DescriptorPoolMetal()
{
}

DescriptorPoolMetal::~DescriptorPoolMetal()
{
}

HS_NS_END
