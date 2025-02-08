//
//  RHIDefinition
//  HSMR
//
//  Created by Yongsik Im on 2/6/25.
//
#ifndef __HS_RENDER_DEFINITION_H__
#define __HS_RENDER_DEFINITION_H__

#include "Precompile.h"

#include <vector>

HS_NS_BEGIN

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
    TEX_1D       = 0,
    TEX_1D_ARRAY = 1,
    TEX_2D       = 2,
    TEX_2D_ARRAY = 3,
    TEX_CUBE     = 5,
};

enum class ETextureUsage
{
    UNKNOWN          = 0x0000,
    SHADER_READ      = 0x0001,
    SHADER_WRITE     = 0x0002,
    RENDER_TARGET    = 0x0004,
    PIXELFORMAT_VIEW = 0x0010,
};

struct RenderParameter
{
    // Render()호출에 던질 것들 전부 넣기
};

struct Texture
{
    EPixelFormat  format      = EPixelFormat::R8G8B8A8_UNORM;
    ETextureType  type        = ETextureType::TEX_2D;
    ETextureUsage usage       = ETextureUsage::UNKNOWN;
    uint32        width       = 0;
    uint32        height      = 0;
    uint32        depth       = 1;
    uint32        mipLevel    = 1;
    uint32        arrayLength = 1;

    bool  isSwapchianTexture = false;
    void* handle             = nullptr;
};

struct RenderTexture
{
    uint32 width;
    uint32 height;

    std::vector<Texture*> colorBuffers;
    Texture*              depthStencilBuffer;
};

struct RenderCommandEncoder
{
    
    
};

HS_NS_END

#endif
