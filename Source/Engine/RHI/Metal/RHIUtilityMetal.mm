#include "Engine/RHI/Metal/RHIUtilityMetal.h"

#include "Engine/Core/Log.h"

#import <Foundation/Foundation.h>

HS_NS_BEGIN

MTLPixelFormat hs_rhi_to_pixel_format(EPixelFormat format)
{
    switch (format)
    {
        case EPixelFormat::R8G8B8A8_UNORM:
            return MTLPixelFormatRGBA8Unorm;
        case EPixelFormat::R8G8B8A8_SRGB:
            return MTLPixelFormatRGBA8Unorm_sRGB;
        case EPixelFormat::B8G8A8R8_UNORM:
            return MTLPixelFormatBGRA8Unorm;
        case EPixelFormat::B8G8A8R8_SRGB:
            return MTLPixelFormatBGRA8Unorm_sRGB;

        default:
            break;
    }

    HS_LOG(crash, "Unsupported MTLPixelFormat!");
    return MTLPixelFormatInvalid;
}

EPixelFormat hs_rhi_from_pixel_format(MTLPixelFormat format)
{
    switch (format)
    {
        case MTLPixelFormatRGBA8Unorm:
            return EPixelFormat::R8G8B8A8_UNORM;
        case MTLPixelFormatRGBA8Unorm_sRGB:
            return EPixelFormat::R8G8B8A8_SRGB;
        case MTLPixelFormatBGRA8Unorm:
            return EPixelFormat::B8G8A8R8_UNORM;
        case MTLPixelFormatBGRA8Unorm_sRGB:
            return EPixelFormat::B8G8A8R8_SRGB;

        default:
            break;
    }

    HS_LOG(crash, "Unsupported EPixelFormat!");
    return EPixelFormat::INVALID;
}

MTLLoadAction hs_rhi_to_load_action(ELoadAction action)
{
    switch (action)
    {
        case ELoadAction::DONT_CARE:
            return MTLLoadActionDontCare;
        case ELoadAction::LOAD:
            return MTLLoadActionLoad;
        case ELoadAction::CLEAR:
            return MTLLoadActionClear;
            
        default:
            break;
    }
    HS_LOG(crash, "Unsupported MTLLoadAction");
    return MTLLoadActionDontCare;
}

ELoadAction hs_rhi_from_load_action(MTLLoadAction action)
{
    switch (action)
    {
        case MTLLoadActionDontCare:
            return ELoadAction::DONT_CARE;
        case MTLLoadActionLoad:
            return ELoadAction::LOAD;
        case MTLLoadActionClear:
            return ELoadAction::CLEAR;

        default:
            break;
    }
    HS_LOG(crash, "Unsupported ELoadAction");
    return ELoadAction::INVALID;
}

MTLStoreAction hs_rhi_to_store_action(EStoreAction action)
{
    switch (action)
    {
        case EStoreAction::DONT_CARE:
            return MTLStoreActionDontCare;
        case EStoreAction::STORE:
            return MTLStoreActionStore;

        default:
            break;
    }
    HS_LOG(crash, "Unsupported MTLStoreAction");
    return MTLStoreActionUnknown;
}

EStoreAction hs_rhi_from_store_action(MTLStoreAction action)
{
    switch (action)
    {
        case MTLStoreActionDontCare:
            return EStoreAction::DONT_CARE;
        case MTLStoreActionStore:
            return EStoreAction::STORE;

        default:
            break;
    }
    HS_LOG(crash, "Unsupported EStoreAction");
    return EStoreAction::INVALID;
}

MTLViewport hs_rhi_to_viewport(Viewport vp)
{
    return (MTLViewport){vp.x, vp.y, vp.width, vp.height, vp.zMear, vp.zFar};
}

Viewport hs_rhi_from_viewport(MTLViewport vp)
{
    return Viewport{
        static_cast<float>(vp.originX),
        static_cast<float>(vp.originY),
        static_cast<float>(vp.width),
        static_cast<float>(vp.height),
        static_cast<float>(vp.znear),
        static_cast<float>(vp.zfar)
    };
}

MTLTextureUsage hs_rhi_to_texture_usage(ETextureUsage usage)
{
    switch(usage)
    {
        case ETextureUsage::SHADER_READ:
            return MTLTextureUsageShaderRead;
        case ETextureUsage::SHADER_WRITE:
            return MTLTextureUsageShaderWrite;
        case ETextureUsage::PIXELFORMAT_VIEW:
            return MTLTextureUsagePixelFormatView;
        case ETextureUsage::RENDER_TARGET:
            return MTLTextureUsageRenderTarget;
            
        default:
            break;
    }
    HS_LOG(crash, "Unsupported MTLTextureUsage");
    return MTLTextureUsageUnknown;
}

ETextureUsage hs_rhi_from_texture_usage(MTLTextureUsage usage)
{
    switch(usage)
    {
        case MTLTextureUsageShaderRead:
            return ETextureUsage::SHADER_READ;
        case MTLTextureUsageShaderWrite:
            return ETextureUsage::SHADER_WRITE;
        case MTLTextureUsagePixelFormatView:
            return ETextureUsage::PIXELFORMAT_VIEW;
        case MTLTextureUsageRenderTarget:
            return ETextureUsage::RENDER_TARGET;
            
        default:
            break;
    }
    HS_LOG(crash, "Unsupported ETextureUsage");
    return ETextureUsage::UNKNOWN;
}

MTLTextureType hs_rhi_to_texture_type(ETextureType type)
{
    switch(type)
    {
        case ETextureType::TEX_1D:
            return MTLTextureType1D;
        case ETextureType::TEX_1D_ARRAY:
            return MTLTextureType1DArray;
        case ETextureType::TEX_2D:
            return MTLTextureType2D;
        case ETextureType::TEX_2D_ARRAY:
            return MTLTextureType2DArray;
        case ETextureType::TEX_CUBE:
            return MTLTextureTypeCube;
            
        default:
            break;
    }
    HS_LOG(crash, "Unsupported MTLTextureType");
    return MTLTextureType2D;
}

ETextureType hs_rhi_from_texture_type(MTLTextureType type)
{
    switch(type)
    {
        case MTLTextureType1D:
            return ETextureType::TEX_1D;
        case MTLTextureType1DArray:
            return ETextureType::TEX_1D_ARRAY;
        case MTLTextureType2D:
            return ETextureType::TEX_2D;
        case MTLTextureType2DArray:
            return ETextureType::TEX_2D_ARRAY;
        case MTLTextureTypeCube:
            return ETextureType::TEX_CUBE;
            
        default:
            break;
    }
    HS_LOG(crash, "Unsupported ETextureType");
    return ETextureType::INVALID;
}

MTLClearColor hs_rhi_to_clear_color(const float* color)
{
    double r = static_cast<double>(color[0]);
    double g = static_cast<double>(color[1]);
    double b = static_cast<double>(color[2]);
    double a = static_cast<double>(color[3]);
    
    return MTLClearColorMake(r, g, b, a);
}
    
void hs_rhi_from_clear_color(MTLClearColor color, float* outColor)
{
    outColor[0] = static_cast<float>(color.red);
    outColor[1] = static_cast<float>(color.green);
    outColor[2] = static_cast<float>(color.blue);
    outColor[3] = static_cast<float>(color.alpha);
}

HS_NS_END
