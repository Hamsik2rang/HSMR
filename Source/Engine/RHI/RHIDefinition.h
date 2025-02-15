//
//  RHIDefinition
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_RHI_DEFINITION_H__
#define __HS_RHI_DEFINITION_H__

#include "Precompile.h"

#include "Engine/Core/Log.h"

#include <vector>

HS_NS_BEGIN

class RHIHandle
{
public:
    enum class EType
    {
        BUFFER,
        TEXTURE,
        SAMPLER,
        SHADER,
        RESOURCE_LAYOUT,
        DESCRIPTOR_SET,
        DESCRIPTOR_POOL,
        RENDER_PASS,
        FRAMEBUFFER,
        GRAPHICS_PIPELINE,
        COMPUTE_PIPELINE,
        COMMAND_QUEUE,
        COMMAND_POOL,
        COMMAND_BUFFER,
        FENCE,
        SEMAPHORE,
        RESOURCE_BARRIER
    };
    
    RHIHandle() = delete;
    RHIHandle(RHIHandle::EType type) : _type(type) {}
    virtual ~RHIHandle() {}

    RHIHandle::EType GetType() const { return _type;}
    void GetHash() const { return _hash; }
    int Retain()
    {
        return ++_refs;
    }
    
    int Release()
    {
        if(_refs <=0)
        {
            HS_LOG(crash, "Over Released");
        }
        
        if(--_refs == 0)
        {
            // add pending delete list
        }
        
        return _refs;
    }
    
protected:
    const EType _type;
    
    int _refs;
    uint32 _hash;
    //...
    
};

enum class EPixelFormat
{
    INVALID = 0,

    R8G8B8A8_UNORM = 70,
    R8G8B8A8_SRGB  = 71,
    B8G8A8R8_UNORM = 80,
    B8G8A8R8_SRGB  = 81,

    DEPTH32          = 252,
    STENCIL8         = 253,
    DEPTH24_STENCIL8 = 255,
    DEPTH32_STENCIL8 = 260,
};

enum class ETextureType
{
    INVALID = 0,

    TEX_1D,
    TEX_1D_ARRAY,
    TEX_2D,
    TEX_2D_ARRAY,
    TEX_CUBE,
};

enum class ETextureUsage
{
    UNKNOWN          = 0x0000,
    SHADER_READ      = 0x0001,
    SHADER_WRITE     = 0x0002,
    RENDER_TARGET    = 0x0004,
    PIXELFORMAT_VIEW = 0x0010,
};

struct TextureInfo
{
    EPixelFormat  format = EPixelFormat::R8G8B8A8_UNORM;
    ETextureType  type   = ETextureType::TEX_2D;
    ETextureUsage usage  = ETextureUsage::UNKNOWN;
    struct
    {
        uint32 width  = 0;
        uint32 height = 0;
        uint32 depth  = 1;
    } extent;

    uint32 mipLevel    = 1;
    uint32 arrayLength = 1;
    size_t byteSize    = 0;

    bool isCompressed         = false;
    bool isSwapchianTexture   = false;
    bool isDepthStencilBuffer = false;
    bool useGenerateMipmap    = false;
};

enum class EFilterMode
{
    INVALID = 0,

    NEAREST,
    LINEAR
};

enum class EAddressMode
{
    INVALID = 0,

    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
};

struct SamplerInfo
{
    ETextureType type;
    EFilterMode  minFilter;
    EFilterMode  magFilter;
    EFilterMode  mipFilter;
    EAddressMode addressU;
    EAddressMode addressV;
    EAddressMode addressW;

    bool isPixelCoordinate = false;
};

enum class EBufferUsage
{
    INVALID = 0,

    UNIFORM = 0x00000010,
    INDEX   = 0x00000040,
    VERTEX  = 0x00000080,
    TEXEL   = 0x00000004,
};

enum class EBufferMemoryOption
{
    INVALID = 0,

    NOTHING,
    MAPPED,
    STATIC,
    DYNAMIC
};

struct BufferInfo
{
    EBufferUsage usage;
    EBufferMemoryOption memoryOption;
};

class Texture;
class RenderPass;

struct RenderTexture
{
    EPixelFormat format;
    uint32       width;
    uint32       height;

    std::vector<Texture*> colorBuffers;
    Texture*              depthStencilBuffer;
};

struct SwapchainInfo
{
    bool useDepth;
    bool useStencil;
    bool useMSAA;

    void* nativeWindowHandle;
};

enum class EStoreAction
{
    INVALID = 0,

    DONT_CARE,
    STORE,
};

enum class ELoadAction
{
    INVALID = 0,

    DONT_CARE,
    LOAD,
    CLEAR,
};

struct ClearValue
{
    union
    {
        float color[4];
        struct
        {
            float depth   = 1.0f;
            float stencil = 0.0f;
        };
    };
};

struct Attachment
{
    EPixelFormat format;
    ELoadAction  loadAction;
    EStoreAction storeAction;
    ClearValue   clearValue;
    bool         isDepthStencil = false;
};

struct RenderPassInfo
{
    std::vector<Attachment> colorAttachments;
    Attachment              depthStencilAttachment;
    size_t                  colorAttachmentCount;

    bool useDepthStencilAttachment = false;
    bool isSwapchainRenderPass     = false;
};

class RenderPass;

struct FramebufferInfo
{
    RenderPass*           renderPass;
    std::vector<Texture*> colorBuffers;
    Texture*              depthStencilBuffer;
    Texture*              resolveBuffer;

    uint32 width                  = 1;
    uint32 height                 = 1;
    bool   isSwapchainFramebuffer = false;
};

struct GraphicsPipelineInfo
{
    //...
};

struct ComputePipelineInfo
{
    //...
    
};

enum class EShaderParameterType
{
    T_BOOL = 0,
    T_CHAR,
    T_INT8,
    T_UINT8,
    T_INT16,
    T_UINT16,
    T_INT32,
    T_UINT32,
    T_INT64,
    T_UINT64,

    T_HALF,
    T_FLOAT,
    T_DOUBLE,

    T_VEC2,
    T_VEC3,
    T_VEC4,

    T_MAT22,
    T_MAT33,
    T_MAT44,

    T_STRUCT // Constant Buffer
    //,..
};

struct ShaderParameterValue
{
    EShaderParameterType type;

    void* data = nullptr;
    uint8 offset;
    uint8 align;
};

struct Viewport
{
    float x      = 0;
    float y      = 0;
    float width  = 0;
    float height = 0;
    float zMear  = 0.0f;
    float zFar   = 1.0f;
};

#ifdef DOMAIN
#pragma push_macro("DOMAIN")
#undef DOMAIN
#endif
enum class EShaderStage
{
    VERTEX,
    GEOMETRY,
    DOMAIN,
    HULL,
    FRAGMENT,

    COMPUTE
};

#ifdef DOMAIN
#pragma pop_macro("DOMAIN")
#endif


struct ShaderInfo
{
    EShaderStage stage;
    const char* entryName;
    
    bool isBuiltIn = true;
};


enum class EResourceType : uint8
{
    SAMPLER,
    COMBINED_IMAGE_SAMPLER,
    SAMPLED_IMAGE,
    STORAGE_IMAGE,
    UNIFORM_TEXEL_BUFFER,
    STORAGET_TEXEL_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    UNIFORM_BUFFER_DYNAMIC,
    STORAGE_BUFFER_DYNAMIC,
    INPUT_ATTACHMENT,
};

struct ResourceBinding
{
    EResourceType type;
    EShaderStage stage;
    uint8 biding;
    uint8 arrayCount;
    
    std::string name;
    uint32 nameHash;
};

HS_NS_END

#endif
