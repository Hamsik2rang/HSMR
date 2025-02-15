//
//  ResourceHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/9/25.
//
#ifndef __HS_RESOURCE_HANDLE_H__
#define __HS_RESOURCE_HANDLE_H__

#include "Precompile.h"

#include "Engine/RHI/RenderDefinition.h"
#include <simd/simd.h>

HS_NS_BEGIN

class Texture : public RHIHandle
{
public:
    Texture(void* image, const TextureInfo& info);
    Texture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage);
    ~Texture() override;

    TextureInfo GetInfo() { return _info;}
private:
    TextureInfo _info;
};

class Sampler : public RHIHandle
{
public:
    

    SamplerInfo GetInfo() { return _info; }
private:
    SamplerInfo _info;

};


class Shader
{
public:
    Shader(EShaderStage stage, const char* path, const char* entryName, bool isBuiltIn = true);
    Shader(EShaderStage stage, const char* byteCocde, size_t byteSize, const char* entryName, bool isBuitIn = true);

    virtual ~Shader();

    const char*  GetByteCode() { return _byteCode; }
    const char*  GetEntryName() { return _entryName; }
    size_t       GetByteSize() { return _byteSize; }
    EShaderStage GetStage() { return _stage; }

private:
    void loadShader();

    bool         _isBuiltIn;
    const char*  _byteCode;
    const char*  _entryName;
    size_t       _byteSize;
    EShaderStage _stage;
};


class Buffer : public RHIHandle
{
public:
    Buffer(void* data, size_t byteSize, EBufferUsage usage, EBufferMemoryOption memoryOption);
    ~Buffer() override;

    EBufferUsage        _usage;
    EBufferMemoryOption _memoryOption;
    void*               byte;
    size_t              byteSize;
};

class ResourceLayout : public RHIHandle
{
public:
    
    
    std::vector<ResourceBinding> GetBinding();
private:
    std::vector<ResourceBinding> _bindings;
    uint32 hash;
};

class DescriptorSet : public RHIHandle
{
    std::vector<ResourceLayout> _layouts;
    
};

class DescriptorPool : public RHIHandle
{
    
};

HS_NS_END

#endif /* __HS_RESOURCE_HANDLE_H__ */
