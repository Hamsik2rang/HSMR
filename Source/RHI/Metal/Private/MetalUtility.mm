#include "RHI/Metal/MetalUtility.h"

#include "Core/Log.h"

#import <Foundation/Foundation.h>

HS_NS_BEGIN

MTLPixelFormat MetalUtility::ToPixelFormat(EPixelFormat format)
{
    switch (format)
    {
        case EPixelFormat::R8G8B8A8_UNORM:   return MTLPixelFormatRGBA8Unorm;
        case EPixelFormat::R8G8B8A8_SRGB:    return MTLPixelFormatRGBA8Unorm_sRGB;
        case EPixelFormat::B8G8A8R8_UNORM:   return MTLPixelFormatBGRA8Unorm;
        case EPixelFormat::B8G8A8R8_SRGB:    return MTLPixelFormatBGRA8Unorm_sRGB;

        case EPixelFormat::DEPTH32:          return MTLPixelFormatDepth32Float;
        case EPixelFormat::DEPTH32_STENCIL8: return MTLPixelFormatDepth32Float_Stencil8;
        case EPixelFormat::DEPTH24_STENCIL8: return MTLPixelFormatDepth24Unorm_Stencil8;

        default:                             break;
    }

    HS_LOG(crash, "Unsupported EPixelFormat!");
    return MTLPixelFormatInvalid;
}

EPixelFormat MetalUtility::FromPixelFormat(MTLPixelFormat format)
{
    switch (format)
    {
        case MTLPixelFormatRGBA8Unorm:      return EPixelFormat::R8G8B8A8_UNORM;
        case MTLPixelFormatRGBA8Unorm_sRGB: return EPixelFormat::R8G8B8A8_SRGB;
        case MTLPixelFormatBGRA8Unorm:      return EPixelFormat::B8G8A8R8_UNORM;
        case MTLPixelFormatBGRA8Unorm_sRGB: return EPixelFormat::B8G8A8R8_SRGB;

        default:                            break;
    }

    HS_LOG(crash, "Unsupported MTLPixelFormat!");
    return EPixelFormat::INVALID;
}

MTLVertexFormat MetalUtility::ToVertexFormat(EVertexFormat format)
{
    switch (format)
    {

        case EVertexFormat::FLOAT:  return MTLVertexFormatFloat;
        case EVertexFormat::FLOAT2: return MTLVertexFormatFloat2;
        case EVertexFormat::FLOAT3: return MTLVertexFormatFloat3;
        case EVertexFormat::FLOAT4: return MTLVertexFormatFloat4;
        case EVertexFormat::HALF:   return MTLVertexFormatHalf;
        case EVertexFormat::HALF2:  return MTLVertexFormatHalf2;
        case EVertexFormat::HALF3:  return MTLVertexFormatHalf3;
        case EVertexFormat::HALF4:  return MTLVertexFormatHalf4;
     
        default:                    break;
    }
    
    HS_LOG(crash, "Unsupported EVertexFormat!");
    return MTLVertexFormatInvalid;
}

EVertexFormat MetalUtility::FromVertexFormat(MTLVertexFormat format)
{
    switch (format)
    {

        case MTLVertexFormatFloat:  return EVertexFormat::FLOAT;
        case MTLVertexFormatFloat2: return EVertexFormat::FLOAT2;
        case MTLVertexFormatFloat3: return EVertexFormat::FLOAT3;
        case MTLVertexFormatFloat4: return EVertexFormat::FLOAT4;
        case MTLVertexFormatHalf:   return EVertexFormat::HALF;
        case MTLVertexFormatHalf2:  return EVertexFormat::HALF2;
        case MTLVertexFormatHalf3:  return EVertexFormat::HALF3;
        case MTLVertexFormatHalf4:  return EVertexFormat::HALF4;

        default:                    break;
    }
    
    HS_LOG(crash, "Unsupported EVertexFormat!");
    return EVertexFormat::INVALID;
}
 
MTLLoadAction MetalUtility::ToLoadAction(ELoadAction action)
{
    switch (action)
    {
        case ELoadAction::DONT_CARE: return MTLLoadActionDontCare;
        case ELoadAction::LOAD:      return MTLLoadActionLoad;
        case ELoadAction::CLEAR:     return MTLLoadActionClear;

        default:                     break;
    }
    HS_LOG(crash, "Unsupported ELoadAction");
    return MTLLoadActionDontCare;
}

ELoadAction MetalUtility::FromLoadAction(MTLLoadAction action)
{
    switch (action)
    {
        case MTLLoadActionDontCare: return ELoadAction::DONT_CARE;
        case MTLLoadActionLoad:     return ELoadAction::LOAD;
        case MTLLoadActionClear:    return ELoadAction::CLEAR;

        default:                    break;
    }
    HS_LOG(crash, "Unsupported MTLLoadAction");
    return ELoadAction::INVALID;
}

MTLStoreAction MetalUtility::ToStoreAction(EStoreAction action)
{
    switch (action)
    {
        case EStoreAction::DONT_CARE: return MTLStoreActionDontCare;
        case EStoreAction::STORE:     return MTLStoreActionStore;

        default:                      break;
    }
    HS_LOG(crash, "Unsupported EStoreAction");
    return MTLStoreActionUnknown;
}

EStoreAction MetalUtility::FromStoreAction(MTLStoreAction action)
{
    switch (action)
    {
        case MTLStoreActionDontCare: return EStoreAction::DONT_CARE;
        case MTLStoreActionStore:    return EStoreAction::STORE;

        default:                     break;
    }
    HS_LOG(crash, "Unsupported MTLStoreAction");
    return EStoreAction::INVALID;
}

MTLViewport MetalUtility::ToViewport(Viewport vp)
{
    return (MTLViewport){vp.x, vp.y, vp.width, vp.height, vp.zNear, vp.zFar};
}

Viewport MetalUtility::FromViewport(MTLViewport vp)
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

MTLTextureUsage MetalUtility::ToTextureUsage(ETextureUsage usage)
{
    MTLTextureUsage result = 0;
    if ((usage & ETextureUsage::SAMPLED) != 0) result |= MTLTextureUsageShaderRead;
    if ((usage & ETextureUsage::STAGING) != 0) result |= MTLTextureUsageShaderWrite;
    if ((usage & ETextureUsage::COLOR_ATTACHMENT) != 0) result |= MTLTextureUsageRenderTarget;
    if ((usage & ETextureUsage::DEPTH_STENCIL_ATTACHMENT) != 0) result |= MTLTextureUsageRenderTarget;

    return result;
}

ETextureUsage MetalUtility::FromTextureUsage(MTLTextureUsage usage)
{
    ETextureUsage result = ETextureUsage::UNKNOWN;
    if ((usage & MTLTextureUsageShaderRead) != 0) result |= ETextureUsage::SAMPLED;
    if ((usage & MTLTextureUsageShaderWrite) != 0) result |= ETextureUsage::STAGING;
    if ((usage & MTLTextureUsageRenderTarget) != 0) result |= (ETextureUsage::COLOR_ATTACHMENT | ETextureUsage::DEPTH_STENCIL_ATTACHMENT);
    
    return result;
}

MTLTextureType MetalUtility::ToTextureType(ETextureType type)
{
    switch (type)
    {
        case ETextureType::TEX_1D:       return MTLTextureType1D;
        case ETextureType::TEX_1D_ARRAY: return MTLTextureType1DArray;
        case ETextureType::TEX_2D:       return MTLTextureType2D;
        case ETextureType::TEX_2D_ARRAY: return MTLTextureType2DArray;
        case ETextureType::TEX_CUBE:     return MTLTextureTypeCube;
        case ETextureType::TEX_3D:       return MTLTextureType3D;

        default:                         break;
    }
    HS_LOG(crash, "Unsupported ETextureType");
    return MTLTextureType2D;
}

ETextureType MetalUtility::FromTextureType(MTLTextureType type)
{
    switch (type)
    {
        case MTLTextureType1D:      return ETextureType::TEX_1D;
        case MTLTextureType1DArray: return ETextureType::TEX_1D_ARRAY;
        case MTLTextureType2D:      return ETextureType::TEX_2D;
        case MTLTextureType2DArray: return ETextureType::TEX_2D_ARRAY;
        case MTLTextureTypeCube:    return ETextureType::TEX_CUBE;
        case MTLTextureType3D:      return ETextureType::TEX_3D;

        default:                    break;
    }
    HS_LOG(crash, "Unsupported MTLTextureType");
    return ETextureType::INVALID;
}

size_t MetalUtility::GetBytesPerPixel(EPixelFormat format)
{
    return GetBytesPerPixel(ToPixelFormat(format));
}

size_t MetalUtility::GetBytesPerPixel(MTLPixelFormat format)
{
    switch (format)
    {
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatBGRA8Unorm:
        case MTLPixelFormatBGRA8Unorm_sRGB:
            return 4;

        default:
            break;
    }

    HS_LOG(crash, "Unsupported MTLPixelFormat");
    return 0;
}

MTLBlendFactor MetalUtility::ToBlendFactor(EBlendFactor factor)
{
    switch (factor)
    {
        case EBlendFactor::ZERO:                 return MTLBlendFactorZero;
        case EBlendFactor::ONE:                  return MTLBlendFactorOne;
        case EBlendFactor::SRC_COLOR:            return MTLBlendFactorSourceColor;
        case EBlendFactor::ONE_MINUS_SRC_COLOR:  return MTLBlendFactorOneMinusSourceColor;
        case EBlendFactor::DST_COLOR:            return MTLBlendFactorDestinationColor;
        case EBlendFactor::ONE_MINUS_DST_COLOR:  return MTLBlendFactorOneMinusDestinationColor;
        case EBlendFactor::SRC_ALPHA:            return MTLBlendFactorSourceAlpha;
        case EBlendFactor::ONE_MINUS_SRC_ALPHA:  return MTLBlendFactorOneMinusSourceAlpha;
        case EBlendFactor::DST_ALPHA:            return MTLBlendFactorDestinationAlpha;
        case EBlendFactor::ONE_MINUS_DST_ALPHA:  return MTLBlendFactorOneMinusDestinationAlpha;

        case EBlendFactor::SRC_ALPHA_SATURATE:   return MTLBlendFactorSourceAlphaSaturated;
        case EBlendFactor::SRC1_COLOR:           return MTLBlendFactorSource1Color;
        case EBlendFactor::ONE_MINUS_SRC1_COLOR: return MTLBlendFactorOneMinusSource1Color;
        case EBlendFactor::SRC1_ALPHA:           return MTLBlendFactorSource1Alpha;
        case EBlendFactor::ONE_MINUS_SRC1_ALPHA: return MTLBlendFactorOneMinusSource1Alpha;

        default:                                 break;
    }

    HS_LOG(crash, "Unsupported EBlendFactor");
    return MTLBlendFactorZero;
}

EBlendFactor MetalUtility::FromBlendFactor(MTLBlendFactor factor)
{
    switch (factor)
    {
        case MTLBlendFactorZero:                     return EBlendFactor::ZERO;
        case MTLBlendFactorOne:                      return EBlendFactor::ONE;
        case MTLBlendFactorSourceColor:              return EBlendFactor::SRC_COLOR;
        case MTLBlendFactorOneMinusSourceColor:      return EBlendFactor::ONE_MINUS_SRC_COLOR;
        case MTLBlendFactorDestinationColor:         return EBlendFactor::DST_COLOR;
        case MTLBlendFactorOneMinusDestinationColor: return EBlendFactor::ONE_MINUS_DST_COLOR;
        case MTLBlendFactorSourceAlpha:              return EBlendFactor::SRC_ALPHA;
        case MTLBlendFactorOneMinusSourceAlpha:      return EBlendFactor::ONE_MINUS_SRC_ALPHA;
        case MTLBlendFactorDestinationAlpha:         return EBlendFactor::DST_ALPHA;
        case MTLBlendFactorOneMinusDestinationAlpha: return EBlendFactor::ONE_MINUS_DST_ALPHA;

        case MTLBlendFactorSourceAlphaSaturated:     return EBlendFactor::SRC_ALPHA_SATURATE;
        case MTLBlendFactorSource1Color:             return EBlendFactor::SRC1_COLOR;
        case MTLBlendFactorOneMinusSource1Color:     return EBlendFactor::ONE_MINUS_SRC1_COLOR;
        case MTLBlendFactorSource1Alpha:             return EBlendFactor::SRC1_ALPHA;
        case MTLBlendFactorOneMinusSource1Alpha:     return EBlendFactor::ONE_MINUS_SRC1_ALPHA;

        default:                                     break;
    }

    HS_LOG(crash, "Unsupported EBlendFactor");
    return EBlendFactor::INVALID;
}

MTLBlendOperation MetalUtility::ToBlendOperation(EBlendOp operation)
{
    switch (operation)
    {
        case EBlendOp::ADD:              return MTLBlendOperationAdd;
        case EBlendOp::SUBTRACT:         return MTLBlendOperationSubtract;
        case EBlendOp::REVERSE_SUBTRACT: return MTLBlendOperationReverseSubtract;
        case EBlendOp::MAX:              return MTLBlendOperationMax;
        case EBlendOp::MIN:              return MTLBlendOperationMin;

        default:                         break;
    }

    HS_LOG(crash, "Unsupported EBlendOp");
    return MTLBlendOperationAdd;
}

EBlendOp MetalUtility::FromBlendOperation(MTLBlendOperation operation)
{
    switch (operation)
    {
        case MTLBlendOperationAdd:             return EBlendOp::ADD;
        case MTLBlendOperationSubtract:        return EBlendOp::SUBTRACT;
        case MTLBlendOperationReverseSubtract: return EBlendOp::REVERSE_SUBTRACT;
        case MTLBlendOperationMax:             return EBlendOp::MAX;
        case MTLBlendOperationMin:             return EBlendOp::MIN;

        default:                               break;
    }

    HS_LOG(crash, "Unsupported MTLBlendOperation");
    return EBlendOp::INVALID;
}

MTLCompareFunction MetalUtility::ToCompareFunction(ECompareOp compare)
{
    switch (compare)
    {
        case ECompareOp::NEVER:            return MTLCompareFunctionNever;
        case ECompareOp::LESS:             return MTLCompareFunctionLess;
        case ECompareOp::EQUAL:            return MTLCompareFunctionEqual;
        case ECompareOp::LESS_OR_EQUAL:    return MTLCompareFunctionLessEqual;
        case ECompareOp::GREATER:          return MTLCompareFunctionGreater;
        case ECompareOp::NOT_EQUAL:        return MTLCompareFunctionNotEqual;
        case ECompareOp::GREATER_OR_EQUAL: return MTLCompareFunctionGreaterEqual;
        case ECompareOp::ALWAYS:           return MTLCompareFunctionAlways;

        default:                           break;
    }

    HS_LOG(crash, "Unsupported ECompareOp");
    return MTLCompareFunctionNever;
}

ECompareOp MetalUtility::FromCompareFunction(MTLCompareFunction compare)
{
    switch (compare)
    {
        case MTLCompareFunctionNever:        return ECompareOp::NEVER;
        case MTLCompareFunctionLess:         return ECompareOp::LESS;
        case MTLCompareFunctionEqual:        return ECompareOp::EQUAL;
        case MTLCompareFunctionLessEqual:    return ECompareOp::LESS_OR_EQUAL;
        case MTLCompareFunctionGreater:      return ECompareOp::GREATER;
        case MTLCompareFunctionNotEqual:     return ECompareOp::NOT_EQUAL;
        case MTLCompareFunctionGreaterEqual: return ECompareOp::GREATER_OR_EQUAL;
        case MTLCompareFunctionAlways:       return ECompareOp::ALWAYS;

        default:                             break;
    }

    HS_LOG(crash, "Unsupported MTLCompareFunction");
    return ECompareOp::NEVER;
}

MTLWinding MetalUtility::ToWinding(EFrontFace frontFace)
{
    switch (frontFace)
    {
        case EFrontFace::CLOCKWISE:         return MTLWindingClockwise;
        case EFrontFace::COUNTER_CLOCKWISE: return MTLWindingCounterClockwise;

        default:                            break;
    }

    HS_LOG(crash, "Unsupported EFrontFace");
    return MTLWindingClockwise;
}

EFrontFace MetalUtility::FromWinding(MTLWinding frontFace)
{
    switch (frontFace)
    {
        case MTLWindingClockwise:        return EFrontFace::CLOCKWISE;
        case MTLWindingCounterClockwise: return EFrontFace::COUNTER_CLOCKWISE;

        default:                         break;
    }
    HS_LOG(crash, "Unsupported MTLWinding");
    return EFrontFace::CLOCKWISE;
}

MTLCullMode MetalUtility::ToCullMode(ECullMode cullMode)
{
    switch (cullMode)
    {
        case ECullMode::FRONT: return MTLCullModeFront;
        case ECullMode::BACK:  return MTLCullModeBack;
        case ECullMode::NONE:  return MTLCullModeNone;

        default:               break;
    }
    HS_LOG(crash, "Unsupported ECullMode");
    return MTLCullModeNone;
}

ECullMode MetalUtility::FromCullMode(MTLCullMode cullMode)
{
    switch (cullMode)
    {
        case MTLCullModeFront: return ECullMode::FRONT;
        case MTLCullModeBack:  return ECullMode::BACK;
        case MTLCullModeNone:  return ECullMode::NONE;

        default:               break;
    }
    HS_LOG(crash, "Unsupported MTLCullMode");
    return ECullMode::NONE;
}

MTLTriangleFillMode MetalUtility::ToPolygonMode(EPolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case EPolygonMode::LINE: return MTLTriangleFillModeLines;
        case EPolygonMode::FILL: return MTLTriangleFillModeFill;

        default:                 break;
    }
    HS_LOG(crash, "Unsupported EPolygonMode");
    return MTLTriangleFillModeFill;
}

EPolygonMode MetalUtility::FromPolygonMode(MTLTriangleFillMode polygonMode)
{
    switch (polygonMode)
    {
        case MTLTriangleFillModeLines: return EPolygonMode::LINE;
        case MTLTriangleFillModeFill:  return EPolygonMode::FILL;

        default:                       break;
    }
    HS_LOG(crash, "Unsupported MTLTriangleFillMode");
    return EPolygonMode::FILL;
}

MTLPrimitiveType MetalUtility::ToPrimitiveTopology(EPrimitiveTopology topology)
{
    switch (topology)
    {
        case EPrimitiveTopology::POINT_LIST:     return MTLPrimitiveTypePoint;
        case EPrimitiveTopology::LINE_LIST:      return MTLPrimitiveTypeLine;
        case EPrimitiveTopology::LINE_STRIP:     return MTLPrimitiveTypeLineStrip;
        case EPrimitiveTopology::TRIANGLE_LIST:  return MTLPrimitiveTypeTriangle;
        case EPrimitiveTopology::TRIANGLE_STRIP: return MTLPrimitiveTypeTriangleStrip;

        default:                                 break;
    }
    HS_LOG(crash, "Unsupported EPimitiveTopology");
    return MTLPrimitiveTypeTriangle;
}

EPrimitiveTopology MetalUtility::FromPrimitiveTopology(MTLPrimitiveType topology)
{
    switch (topology)
    {
        case MTLPrimitiveTypePoint:         return EPrimitiveTopology::POINT_LIST;
        case MTLPrimitiveTypeLine:          return EPrimitiveTopology::LINE_LIST;
        case MTLPrimitiveTypeLineStrip:     return EPrimitiveTopology::LINE_STRIP;
        case MTLPrimitiveTypeTriangle:      return EPrimitiveTopology::TRIANGLE_LIST;
        case MTLPrimitiveTypeTriangleStrip: return EPrimitiveTopology::TRIANGLE_STRIP;

        default:                            break;
    }
    HS_LOG(crash, "Unsupported EPimitiveTopology");
    return EPrimitiveTopology::TRIANGLE_LIST;
}

MTLVertexFormat MetalUtility::GetVertexFormatFromSize(size_t size)
{
    switch (size)
    {
        case 4:  return MTLVertexFormatFloat;
        case 8:  return MTLVertexFormatFloat2;
        case 12: return MTLVertexFormatFloat3;
        case 16: return MTLVertexFormatFloat4;

        default: break;
    }

    HS_LOG(crash, "Unsupported VertexFormat size");
    return MTLVertexFormatInvalid;
}
size_t MetalUtility::GetSizeFromVertexFormat(MTLVertexFormat format)
{
    switch (format)
    {
        case MTLVertexFormatFloat:  return 4;
        case MTLVertexFormatFloat2: return 8;
        case MTLVertexFormatFloat3: return 12;
        case MTLVertexFormatFloat4: return 16;

        default:                    break;
    }

    HS_LOG(crash, "Unsupported MTLVertexFormat");
    return 0;
}

MTLResourceOptions MetalUtility::ToBufferOption(EBufferMemoryOption option)
{
    switch (option)
    {
        case EBufferMemoryOption::STATIC:  return MTLStorageModePrivate;
        case EBufferMemoryOption::DYNAMIC: return MTLStorageModeShared;
        case EBufferMemoryOption::MAPPED:  return MTLStorageModeManaged;

        default:                           break;
    }

    HS_LOG(crash, "Unsupported MTLResourceOption");
    return MTLStorageModeManaged;
}
EBufferMemoryOption MetalUtility::FromBufferOption(MTLResourceOptions option)
{
    switch (option)
    {
        case MTLStorageModePrivate: return EBufferMemoryOption::STATIC;
        case MTLStorageModeShared:  return EBufferMemoryOption::DYNAMIC;
        case MTLStorageModeManaged: return EBufferMemoryOption::MAPPED;

        default:                    break;
    }

    HS_LOG(crash, "Unsupported EBufferMemoryOption");
    return EBufferMemoryOption::INVALID;
}

MTLClearColor MetalUtility::ToClearColor(const float* color)
{
    double r = static_cast<double>(color[0]);
    double g = static_cast<double>(color[1]);
    double b = static_cast<double>(color[2]);
    double a = static_cast<double>(color[3]);

    return MTLClearColorMake(r, g, b, a);
}

void MetalUtility::FromClearColor(MTLClearColor color, float* outColor)
{
    outColor[0] = static_cast<float>(color.red);
    outColor[1] = static_cast<float>(color.green);
    outColor[2] = static_cast<float>(color.blue);
    outColor[3] = static_cast<float>(color.alpha);
}

HS_NS_END
