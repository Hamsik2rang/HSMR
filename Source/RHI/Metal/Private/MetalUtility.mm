#include "RHI/Metal/MetalUtility.h"

#include "Core/Log.h"

#import <Foundation/Foundation.h>

HS_NS_BEGIN

MTLPixelFormat MetalUtility::ToPixelFormat(EPixelFormat format)
{
    switch (format)
    {
    case EPixelFormat::R8G8B8A8Unorm:   return MTLPixelFormatRGBA8Unorm;
    case EPixelFormat::R8G8B8A8Srgb:    return MTLPixelFormatRGBA8Unorm_sRGB;
    case EPixelFormat::B8G8A8R8Unorm:   return MTLPixelFormatBGRA8Unorm;
    case EPixelFormat::B8G8A8R8Srgb:    return MTLPixelFormatBGRA8Unorm_sRGB;

    // Floating-point formats (for HDR, compute, atmosphere LUTs)
    case EPixelFormat::R16f:            return MTLPixelFormatR16Float;
    case EPixelFormat::RG16f:           return MTLPixelFormatRG16Float;
    case EPixelFormat::Rgba16f:         return MTLPixelFormatRGBA16Float;
    case EPixelFormat::R32f:            return MTLPixelFormatR32Float;
    case EPixelFormat::RG32f:           return MTLPixelFormatRG32Float;
    case EPixelFormat::Rgba32f:         return MTLPixelFormatRGBA32Float;

    case EPixelFormat::Depth32:         return MTLPixelFormatDepth32Float;
    case EPixelFormat::Depth32Stencil8: return MTLPixelFormatDepth32Float_Stencil8;
    case EPixelFormat::Depth24Stencil8: return MTLPixelFormatDepth24Unorm_Stencil8;

    default:                            break;
    }

    HS_LOG(crash, "Unsupported EPixelFormat!");
    return MTLPixelFormatInvalid;
}

EPixelFormat MetalUtility::FromPixelFormat(MTLPixelFormat format)
{
    switch (format)
    {
    case MTLPixelFormatRGBA8Unorm:      return EPixelFormat::R8G8B8A8Unorm;
    case MTLPixelFormatRGBA8Unorm_sRGB: return EPixelFormat::R8G8B8A8Srgb;
    case MTLPixelFormatBGRA8Unorm:      return EPixelFormat::B8G8A8R8Unorm;
    case MTLPixelFormatBGRA8Unorm_sRGB: return EPixelFormat::B8G8A8R8Srgb;

    // Floating-point formats
    case MTLPixelFormatR16Float:        return EPixelFormat::R16f;
    case MTLPixelFormatRG16Float:       return EPixelFormat::RG16f;
    case MTLPixelFormatRGBA16Float:     return EPixelFormat::Rgba16f;
    case MTLPixelFormatR32Float:        return EPixelFormat::R32f;
    case MTLPixelFormatRG32Float:       return EPixelFormat::RG32f;
    case MTLPixelFormatRGBA32Float:     return EPixelFormat::Rgba32f;

    default:                            break;
    }

    HS_LOG(crash, "Unsupported MTLPixelFormat!");
    return EPixelFormat::Invalid;
}

MTLVertexFormat MetalUtility::ToVertexFormat(EVertexFormat format)
{
    switch (format)
    {

    case EVertexFormat::Float:  return MTLVertexFormatFloat;
    case EVertexFormat::Float2: return MTLVertexFormatFloat2;
    case EVertexFormat::Float3: return MTLVertexFormatFloat3;
    case EVertexFormat::Float4: return MTLVertexFormatFloat4;
    case EVertexFormat::Half:   return MTLVertexFormatHalf;
    case EVertexFormat::Half2:  return MTLVertexFormatHalf2;
    case EVertexFormat::Half3:  return MTLVertexFormatHalf3;
    case EVertexFormat::Half4:  return MTLVertexFormatHalf4;

    default:                    break;
    }

    HS_LOG(crash, "Unsupported EVertexFormat!");
    return MTLVertexFormatInvalid;
}

EVertexFormat MetalUtility::FromVertexFormat(MTLVertexFormat format)
{
    switch (format)
    {

    case MTLVertexFormatFloat:  return EVertexFormat::Float;
    case MTLVertexFormatFloat2: return EVertexFormat::Float2;
    case MTLVertexFormatFloat3: return EVertexFormat::Float3;
    case MTLVertexFormatFloat4: return EVertexFormat::Float4;
    case MTLVertexFormatHalf:   return EVertexFormat::Half;
    case MTLVertexFormatHalf2:  return EVertexFormat::Half2;
    case MTLVertexFormatHalf3:  return EVertexFormat::Half3;
    case MTLVertexFormatHalf4:  return EVertexFormat::Half4;

    default:                    break;
    }

    HS_LOG(crash, "Unsupported EVertexFormat!");
    return EVertexFormat::Invalid;
}

MTLLoadAction MetalUtility::ToLoadAction(ELoadAction action)
{
    switch (action)
    {
    case ELoadAction::DontCare: return MTLLoadActionDontCare;
    case ELoadAction::Load:     return MTLLoadActionLoad;
    case ELoadAction::Clear:    return MTLLoadActionClear;

    default:                    break;
    }
    HS_LOG(crash, "Unsupported ELoadAction");
    return MTLLoadActionDontCare;
}

ELoadAction MetalUtility::FromLoadAction(MTLLoadAction action)
{
    switch (action)
    {
    case MTLLoadActionDontCare: return ELoadAction::DontCare;
    case MTLLoadActionLoad:     return ELoadAction::Load;
    case MTLLoadActionClear:    return ELoadAction::Clear;

    default:                    break;
    }
    HS_LOG(crash, "Unsupported MTLLoadAction");
    return ELoadAction::Invalid;
}

MTLStoreAction MetalUtility::ToStoreAction(EStoreAction action)
{
    switch (action)
    {
    case EStoreAction::DontCare: return MTLStoreActionDontCare;
    case EStoreAction::Store:    return MTLStoreActionStore;

    default:                     break;
    }
    HS_LOG(crash, "Unsupported EStoreAction");
    return MTLStoreActionUnknown;
}

EStoreAction MetalUtility::FromStoreAction(MTLStoreAction action)
{
    switch (action)
    {
    case MTLStoreActionDontCare: return EStoreAction::DontCare;
    case MTLStoreActionStore:    return EStoreAction::Store;

    default:                     break;
    }
    HS_LOG(crash, "Unsupported MTLStoreAction");
    return EStoreAction::Invalid;
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
        static_cast<float>(vp.zfar)};
}

MTLTextureUsage MetalUtility::ToTextureUsage(ETextureUsage usage)
{
    MTLTextureUsage result = 0;
    if ((usage & ETextureUsage::Sampled) != 0) result |= MTLTextureUsageShaderRead;
    if ((usage & ETextureUsage::Storage) != 0) result |= MTLTextureUsageShaderWrite; // UAV/RWTexture support
    if ((usage & ETextureUsage::ColorAttachment) != 0) result |= MTLTextureUsageRenderTarget;
    if ((usage & ETextureUsage::DepthStencilAttachment) != 0) result |= MTLTextureUsageRenderTarget;

    return result;
}

ETextureUsage MetalUtility::FromTextureUsage(MTLTextureUsage usage)
{
    ETextureUsage result = ETextureUsage::Unknown;
    if ((usage & MTLTextureUsageShaderRead) != 0) result |= ETextureUsage::Sampled;
    if ((usage & MTLTextureUsageShaderWrite) != 0) result |= ETextureUsage::Storage; // Map to STORAGE for compute
    if ((usage & MTLTextureUsageRenderTarget) != 0) result |= (ETextureUsage::ColorAttachment | ETextureUsage::DepthStencilAttachment);

    return result;
}

MTLTextureType MetalUtility::ToTextureType(ETextureType type)
{
    switch (type)
    {
    case ETextureType::Tex1D:      return MTLTextureType1D;
    case ETextureType::Tex1DArray: return MTLTextureType1DArray;
    case ETextureType::Tex2D:      return MTLTextureType2D;
    case ETextureType::Tex2DArray: return MTLTextureType2DArray;
    case ETextureType::TexCube:    return MTLTextureTypeCube;
    case ETextureType::Tex3D:      return MTLTextureType3D;

    default:                       break;
    }
    HS_LOG(crash, "Unsupported ETextureType");
    return MTLTextureType2D;
}

ETextureType MetalUtility::FromTextureType(MTLTextureType type)
{
    switch (type)
    {
    case MTLTextureType1D:      return ETextureType::Tex1D;
    case MTLTextureType1DArray: return ETextureType::Tex1DArray;
    case MTLTextureType2D:      return ETextureType::Tex2D;
    case MTLTextureType2DArray: return ETextureType::Tex2DArray;
    case MTLTextureTypeCube:    return ETextureType::TexCube;
    case MTLTextureType3D:      return ETextureType::Tex3D;

    default:                    break;
    }
    HS_LOG(crash, "Unsupported MTLTextureType");
    return ETextureType::Invalid;
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

    // Floating-point formats
    case MTLPixelFormatR16Float:
        return 2;
    case MTLPixelFormatRG16Float:
    case MTLPixelFormatR32Float:
        return 4;
    case MTLPixelFormatRGBA16Float:
    case MTLPixelFormatRG32Float:
        return 8;
    case MTLPixelFormatRGBA32Float:
        return 16;

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
    case EBlendFactor::Zero:              return MTLBlendFactorZero;
    case EBlendFactor::One:               return MTLBlendFactorOne;
    case EBlendFactor::SrcColor:          return MTLBlendFactorSourceColor;
    case EBlendFactor::OneMinusSrcColor:  return MTLBlendFactorOneMinusSourceColor;
    case EBlendFactor::DstColor:          return MTLBlendFactorDestinationColor;
    case EBlendFactor::OneMinusDstColor:  return MTLBlendFactorOneMinusDestinationColor;
    case EBlendFactor::SrcAlpha:          return MTLBlendFactorSourceAlpha;
    case EBlendFactor::OneMinusSrcAlpha:  return MTLBlendFactorOneMinusSourceAlpha;
    case EBlendFactor::DstAlpha:          return MTLBlendFactorDestinationAlpha;
    case EBlendFactor::OneMinusDstAlpha:  return MTLBlendFactorOneMinusDestinationAlpha;

    case EBlendFactor::SrcAlphaSaturate:  return MTLBlendFactorSourceAlphaSaturated;
    case EBlendFactor::Src1Color:         return MTLBlendFactorSource1Color;
    case EBlendFactor::OneMinusSrc1Color: return MTLBlendFactorOneMinusSource1Color;
    case EBlendFactor::Src1Alpha:         return MTLBlendFactorSource1Alpha;
    case EBlendFactor::OneMinusSrc1Alpha: return MTLBlendFactorOneMinusSource1Alpha;

    default:                              break;
    }

    HS_LOG(crash, "Unsupported EBlendFactor");
    return MTLBlendFactorZero;
}

EBlendFactor MetalUtility::FromBlendFactor(MTLBlendFactor factor)
{
    switch (factor)
    {
    case MTLBlendFactorZero:                     return EBlendFactor::Zero;
    case MTLBlendFactorOne:                      return EBlendFactor::One;
    case MTLBlendFactorSourceColor:              return EBlendFactor::SrcColor;
    case MTLBlendFactorOneMinusSourceColor:      return EBlendFactor::OneMinusSrcColor;
    case MTLBlendFactorDestinationColor:         return EBlendFactor::DstColor;
    case MTLBlendFactorOneMinusDestinationColor: return EBlendFactor::OneMinusDstColor;
    case MTLBlendFactorSourceAlpha:              return EBlendFactor::SrcAlpha;
    case MTLBlendFactorOneMinusSourceAlpha:      return EBlendFactor::OneMinusSrcAlpha;
    case MTLBlendFactorDestinationAlpha:         return EBlendFactor::DstAlpha;
    case MTLBlendFactorOneMinusDestinationAlpha: return EBlendFactor::OneMinusDstAlpha;

    case MTLBlendFactorSourceAlphaSaturated:     return EBlendFactor::SrcAlphaSaturate;
    case MTLBlendFactorSource1Color:             return EBlendFactor::Src1Color;
    case MTLBlendFactorOneMinusSource1Color:     return EBlendFactor::OneMinusSrc1Color;
    case MTLBlendFactorSource1Alpha:             return EBlendFactor::Src1Alpha;
    case MTLBlendFactorOneMinusSource1Alpha:     return EBlendFactor::OneMinusSrc1Alpha;

    default:                                     break;
    }

    HS_LOG(crash, "Unsupported EBlendFactor");
    return EBlendFactor::Invalid;
}

MTLBlendOperation MetalUtility::ToBlendOperation(EBlendOp operation)
{
    switch (operation)
    {
    case EBlendOp::Add:             return MTLBlendOperationAdd;
    case EBlendOp::Subtract:        return MTLBlendOperationSubtract;
    case EBlendOp::ReverseSubtract: return MTLBlendOperationReverseSubtract;
    case EBlendOp::Max:             return MTLBlendOperationMax;
    case EBlendOp::Min:             return MTLBlendOperationMin;

    default:                        break;
    }

    HS_LOG(crash, "Unsupported EBlendOp");
    return MTLBlendOperationAdd;
}

EBlendOp MetalUtility::FromBlendOperation(MTLBlendOperation operation)
{
    switch (operation)
    {
    case MTLBlendOperationAdd:             return EBlendOp::Add;
    case MTLBlendOperationSubtract:        return EBlendOp::Subtract;
    case MTLBlendOperationReverseSubtract: return EBlendOp::ReverseSubtract;
    case MTLBlendOperationMax:             return EBlendOp::Max;
    case MTLBlendOperationMin:             return EBlendOp::Min;

    default:                               break;
    }

    HS_LOG(crash, "Unsupported MTLBlendOperation");
    return EBlendOp::Invalid;
}

MTLCompareFunction MetalUtility::ToCompareFunction(ECompareOp compare)
{
    switch (compare)
    {
    case ECompareOp::Never:          return MTLCompareFunctionNever;
    case ECompareOp::Less:           return MTLCompareFunctionLess;
    case ECompareOp::Equal:          return MTLCompareFunctionEqual;
    case ECompareOp::LessOrEqual:    return MTLCompareFunctionLessEqual;
    case ECompareOp::Greater:        return MTLCompareFunctionGreater;
    case ECompareOp::NotEqual:       return MTLCompareFunctionNotEqual;
    case ECompareOp::GreaterOrEqual: return MTLCompareFunctionGreaterEqual;
    case ECompareOp::Always:         return MTLCompareFunctionAlways;

    default:                         break;
    }

    HS_LOG(crash, "Unsupported ECompareOp");
    return MTLCompareFunctionNever;
}

ECompareOp MetalUtility::FromCompareFunction(MTLCompareFunction compare)
{
    switch (compare)
    {
    case MTLCompareFunctionNever:        return ECompareOp::Never;
    case MTLCompareFunctionLess:         return ECompareOp::Less;
    case MTLCompareFunctionEqual:        return ECompareOp::Equal;
    case MTLCompareFunctionLessEqual:    return ECompareOp::LessOrEqual;
    case MTLCompareFunctionGreater:      return ECompareOp::Greater;
    case MTLCompareFunctionNotEqual:     return ECompareOp::NotEqual;
    case MTLCompareFunctionGreaterEqual: return ECompareOp::GreaterOrEqual;
    case MTLCompareFunctionAlways:       return ECompareOp::Always;

    default:                             break;
    }

    HS_LOG(crash, "Unsupported MTLCompareFunction");
    return ECompareOp::Never;
}

MTLWinding MetalUtility::ToWinding(EFrontFace frontFace)
{
    switch (frontFace)
    {
    case EFrontFace::Clockwise:        return MTLWindingClockwise;
    case EFrontFace::CounterClockwise: return MTLWindingCounterClockwise;

    default:                           break;
    }

    HS_LOG(crash, "Unsupported EFrontFace");
    return MTLWindingClockwise;
}

EFrontFace MetalUtility::FromWinding(MTLWinding frontFace)
{
    switch (frontFace)
    {
    case MTLWindingClockwise:        return EFrontFace::Clockwise;
    case MTLWindingCounterClockwise: return EFrontFace::CounterClockwise;

    default:                         break;
    }
    HS_LOG(crash, "Unsupported MTLWinding");
    return EFrontFace::Clockwise;
}

MTLCullMode MetalUtility::ToCullMode(ECullMode cullMode)
{
    switch (cullMode)
    {
    case ECullMode::Front: return MTLCullModeFront;
    case ECullMode::Back:  return MTLCullModeBack;
    case ECullMode::None:  return MTLCullModeNone;

    default:               break;
    }
    HS_LOG(crash, "Unsupported ECullMode");
    return MTLCullModeNone;
}

ECullMode MetalUtility::FromCullMode(MTLCullMode cullMode)
{
    switch (cullMode)
    {
    case MTLCullModeFront: return ECullMode::Front;
    case MTLCullModeBack:  return ECullMode::Back;
    case MTLCullModeNone:  return ECullMode::None;

    default:               break;
    }
    HS_LOG(crash, "Unsupported MTLCullMode");
    return ECullMode::None;
}

MTLTriangleFillMode MetalUtility::ToPolygonMode(EPolygonMode polygonMode)
{
    switch (polygonMode)
    {
    case EPolygonMode::Line: return MTLTriangleFillModeLines;
    case EPolygonMode::Fill: return MTLTriangleFillModeFill;

    default:                 break;
    }
    HS_LOG(crash, "Unsupported EPolygonMode");
    return MTLTriangleFillModeFill;
}

EPolygonMode MetalUtility::FromPolygonMode(MTLTriangleFillMode polygonMode)
{
    switch (polygonMode)
    {
    case MTLTriangleFillModeLines: return EPolygonMode::Line;
    case MTLTriangleFillModeFill:  return EPolygonMode::Fill;

    default:                       break;
    }
    HS_LOG(crash, "Unsupported MTLTriangleFillMode");
    return EPolygonMode::Fill;
}

MTLPrimitiveType MetalUtility::ToPrimitiveTopology(EPrimitiveTopology topology)
{
    switch (topology)
    {
    case EPrimitiveTopology::PointList:     return MTLPrimitiveTypePoint;
    case EPrimitiveTopology::LineList:      return MTLPrimitiveTypeLine;
    case EPrimitiveTopology::LineStrip:     return MTLPrimitiveTypeLineStrip;
    case EPrimitiveTopology::TriangleList:  return MTLPrimitiveTypeTriangle;
    case EPrimitiveTopology::TriangleStrip: return MTLPrimitiveTypeTriangleStrip;

    default:                                break;
    }
    HS_LOG(crash, "Unsupported EPimitiveTopology");
    return MTLPrimitiveTypeTriangle;
}

EPrimitiveTopology MetalUtility::FromPrimitiveTopology(MTLPrimitiveType topology)
{
    switch (topology)
    {
    case MTLPrimitiveTypePoint:         return EPrimitiveTopology::PointList;
    case MTLPrimitiveTypeLine:          return EPrimitiveTopology::LineList;
    case MTLPrimitiveTypeLineStrip:     return EPrimitiveTopology::LineStrip;
    case MTLPrimitiveTypeTriangle:      return EPrimitiveTopology::TriangleList;
    case MTLPrimitiveTypeTriangleStrip: return EPrimitiveTopology::TriangleStrip;

    default:                            break;
    }
    HS_LOG(crash, "Unsupported EPimitiveTopology");
    return EPrimitiveTopology::TriangleList;
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
    case EBufferMemoryOption::Static:  return MTLResourceStorageModePrivate;
    case EBufferMemoryOption::Dynamic: return MTLResourceStorageModeShared;
    case EBufferMemoryOption::Mapped:  return MTLResourceStorageModeManaged;

    default:                           break;
    }

    HS_LOG(crash, "Unsupported MTLResourceOption");
    return MTLStorageModeManaged;
}
EBufferMemoryOption MetalUtility::FromBufferOption(MTLResourceOptions option)
{
    switch (option)
    {
    case MTLResourceStorageModePrivate: return EBufferMemoryOption::Static;
    case MTLResourceStorageModeShared:  return EBufferMemoryOption::Dynamic;
    case MTLResourceStorageModeManaged: return EBufferMemoryOption::Mapped;

    default:                            break;
    }

    HS_LOG(crash, "Unsupported EBufferMemoryOption");
    return EBufferMemoryOption::Invalid;
}

MTLSamplerMinMagFilter MetalUtility::ToMinMagFilter(EFilterMode filter)
{
    switch (filter)
    {
    case EFilterMode::Nearest: return MTLSamplerMinMagFilterNearest;
    case EFilterMode::Linear:  return MTLSamplerMinMagFilterLinear;
    default:                   break;
    }
    HS_LOG(error, "Unsupported Min/Mag FilterMode!");

    return MTLSamplerMinMagFilterNearest;
}

EFilterMode MetalUtility::FromMinMagFilter(MTLSamplerMinMagFilter filter)
{
    switch (filter)
    {
    case MTLSamplerMinMagFilterNearest: return EFilterMode::Nearest;
    case MTLSamplerMinMagFilterLinear:  return EFilterMode::Linear;
    default:                            break;
    }
    HS_LOG(error, "Unsupported Min/Mag FilterMode!");

    return EFilterMode::Nearest;
}

MTLSamplerMipFilter MetalUtility::ToMipFilter(EFilterMode filter)
{
    switch (filter)
    {
    case EFilterMode::Nearest: return MTLSamplerMipFilterNearest;
    case EFilterMode::Linear:  return MTLSamplerMipFilterLinear;
    default:                   break;
    }
    HS_LOG(error, "Unsupported Mip FilterMode");

    return MTLSamplerMipFilterNearest;
}

EFilterMode MetalUtility::FromMipFilter(MTLSamplerMipFilter filter)
{
    switch (filter)
    {
    case MTLSamplerMipFilterNearest: return EFilterMode::Nearest;
    case MTLSamplerMipFilterLinear:  return EFilterMode::Linear;
    default:                         break;
    }

    HS_LOG(error, "Unsupported Mip FilterMode");

    return EFilterMode::Nearest;
}

MTLSamplerAddressMode MetalUtility::ToSamplerAddressMode(EAddressMode addressMode)
{
    switch (addressMode)
    {
    case EAddressMode::Repeat:            return MTLSamplerAddressModeRepeat;
    case EAddressMode::MirroredRepeat:    return MTLSamplerAddressModeMirrorRepeat;
    case EAddressMode::ClampToEdge:       return MTLSamplerAddressModeClampToEdge;
    case EAddressMode::ClampToBorder:     return MTLSamplerAddressModeClampToBorderColor;
    case EAddressMode::MirrorClampToEdge: return MTLSamplerAddressModeMirrorClampToEdge;
    default:                              break;
    }
    HS_LOG(error, "Unsupported Sampler Address Mode!");

    return MTLSamplerAddressModeClampToEdge;
}

EAddressMode MetalUtility::FromSamplerAddressMode(MTLSamplerAddressMode addressMode)
{
    switch (addressMode)
    {
    case MTLSamplerAddressModeRepeat:             return EAddressMode::Repeat;
    case MTLSamplerAddressModeMirrorRepeat:       return EAddressMode::MirroredRepeat;
    case MTLSamplerAddressModeClampToEdge:        return EAddressMode::ClampToEdge;
    case MTLSamplerAddressModeClampToBorderColor: return EAddressMode::ClampToBorder;
    case MTLSamplerAddressModeMirrorClampToEdge:  return EAddressMode::MirrorClampToEdge;
    default:                                      break;
    }
    HS_LOG(error, "Unsupported Sampler Address Mode!");

    return EAddressMode::ClampToEdge;
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

bool MetalUtility::IsCPUAccessibleBufferOption(EBufferMemoryOption memoryOption)
{
    switch (memoryOption)
    {
    case EBufferMemoryOption::Dynamic:
    case EBufferMemoryOption::Mapped:
        return true;
    default:
        return false;
    }
}

uint32 MetalUtility::GetTextureMipLevelCount(const TextureInfo& info)
{
    if (info.mipLevel > 0)
    {
        return info.mipLevel;
    }

    uint32 maxDim = std::max(info.extent.width, std::max(info.extent.height, info.extent.depth));
    uint32 mipLevelCount = 1;
    while (maxDim > 1)
    {
        maxDim >>= 1;
        ++mipLevelCount;
    }

    return mipLevelCount;
}

HS_NS_END
