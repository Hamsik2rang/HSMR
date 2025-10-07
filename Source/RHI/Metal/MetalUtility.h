//
//  MetalUtility.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RHI_UTILITY_METAL_H__
#define __HS_RHI_UTILITY_METAL_H__

#include "Precompile.h"

#include "RHI/RHIDefinition.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <QuartzCore/CAMetalLayer.h>

HS_NS_BEGIN

class MetalUtility
{
public:
    static MTLPixelFormat ToPixelFormat(EPixelFormat format);
    static EPixelFormat FromPixelFormat(MTLPixelFormat format);
    
    static MTLVertexFormat ToVertexFormat(EVertexFormat format);
    static EVertexFormat FromVertexFormat(MTLVertexFormat format);
    
    static MTLLoadAction ToLoadAction(ELoadAction action);
    static ELoadAction FromLoadAction(MTLLoadAction action);
    
    static MTLStoreAction ToStoreAction(EStoreAction action);
    static EStoreAction FromStoreAction(MTLStoreAction action);
    
    static MTLViewport ToViewport(Viewport vp);
    static Viewport FromViewport(MTLViewport vp);
    
    static MTLTextureUsage ToTextureUsage(ETextureUsage usage);
    static ETextureUsage FromTextureUsage(MTLTextureUsage usage);
    
    static MTLTextureType ToTextureType(ETextureType type);
    static ETextureType FromTextureType(MTLTextureType type);
    
    static MTLBlendFactor ToBlendFactor(EBlendFactor factor);
    static EBlendFactor FromBlendFactor(MTLBlendFactor factor);
    
    static MTLBlendOperation ToBlendOperation(EBlendOp operation);
    static EBlendOp FromBlendOperation(MTLBlendOperation operation);
    
    static MTLCompareFunction ToCompareFunction(ECompareOp compare);
    static ECompareOp FromCompareFunction(MTLCompareFunction compare);
    
    static MTLWinding ToWinding(EFrontFace frontFace);
    static EFrontFace FromWinding(MTLWinding frontFace);
    
    static MTLCullMode ToCullMode(ECullMode cullMode);
    static ECullMode FromCullMode(MTLCullMode cullMode);
    
    static MTLTriangleFillMode ToPolygonMode(EPolygonMode polygonMode);
    static EPolygonMode FromPolygonMode(MTLTriangleFillMode polygonMode);
    
    static MTLPrimitiveType ToPrimitiveTopology(EPrimitiveTopology topology);
    static EPrimitiveTopology FromPrimitiveTopology(MTLPrimitiveType topology);
    
    static size_t GetBytesPerPixel(EPixelFormat format);
    static size_t GetBytesPerPixel(MTLPixelFormat format);
    
    static MTLVertexFormat GetVertexFormatFromSize(size_t size);
    static size_t GetSizeFromVertexFormat(MTLVertexFormat format);
    
    static MTLResourceOptions ToBufferOption(EBufferMemoryOption option);
    static EBufferMemoryOption FromBufferOption(MTLResourceOptions option);
    
    static MTLClearColor ToClearColor(const float* color);
    static void FromClearColor(MTLClearColor color, float* outColor);
};


HS_NS_END

#endif
