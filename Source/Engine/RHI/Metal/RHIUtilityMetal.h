//
//  RHIUtilityMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_RHI_UTILITY_METAL_H__
#define __HS_RHI_UTILITY_METAL_H__

#include "Precompile.h"

#include "Engine/RHI/RHIDefinition.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>
#import <QuartzCore/CAMetalLayer.h>

HS_NS_BEGIN

MTLPixelFormat hs_rhi_to_pixel_format(EPixelFormat format);
EPixelFormat   hs_rhi_from_pixel_format(MTLPixelFormat format);

MTLLoadAction hs_rhi_to_load_action(ELoadAction action);
ELoadAction   hs_rhi_from_laod_action(MTLLoadAction action);

MTLStoreAction hs_rhi_to_store_action(EStoreAction action);
EStoreAction   hs_rhi_from_store_action(MTLStoreAction action);

MTLViewport hs_rhi_to_viewport(Viewport vp);
Viewport    hs_rhi_from_viewport(MTLViewport vp);

MTLTextureUsage hs_rhi_to_texture_usage(ETextureUsage usage);
ETextureUsage   hs_rhi_from_texture_usage(MTLTextureUsage usage);

MTLTextureType hs_rhi_to_texture_type(ETextureType type);
ETextureType   hs_rhi_from_texture_type(MTLTextureType type);

MTLBlendFactor hs_rhi_to_blend_factor(EBlendFactor factor);
EBlendFactor hs_rhi_from_blend_factor(MTLBlendFactor factor);

MTLBlendOperation hs_rhi_to_blend_operation(EBlendOp operation);
EBlendOp hs_rhi_from_blend_operation(MTLBlendOperation operation);

MTLCompareFunction hs_rhi_to_compare_function(ECompareOp compare);
ECompareOp hs_rhi_from_compare_function(MTLCompareFunction compare);

MTLWinding hs_rhi_to_front_face(EFrontFace frontFace);
EFrontFace hs_rhi_from_front_face(MTLWinding frontFace);

MTLCullMode hs_rhi_to_cull_mode(ECullMode cullMode);
ECullMode hs_rhi_from_cull_mode(MTLCullMode cullMode);

MTLTriangleFillMode hs_rhi_to_polygon_mode(EPolygonMode polygonMode);
EPolygonMode hs_rhi_from_polygon_mode(MTLTriangleFillMode polygonMode);

MTLPrimitiveType hs_rhi_to_primitive_topology(EPrimitiveTopology topology);
EPrimitiveTopology hs_rhi_from_primitive_topology(MTLPrimitiveType topology);

size_t hs_rhi_get_bytes_per_pixel(EPixelFormat format);
size_t hs_rhi_get_bytes_per_pixel(MTLPixelFormat format);

MTLVertexFormat hs_rhi_get_vertex_format_from_size(size_t size);
size_t hs_rhi_get_size_from_vertex_format(MTLVertexFormat format);

MTLResourceOptions  hs_rhi_to_buffer_option(EBufferMemoryOption option);
EBufferMemoryOption hs_rhi_from_buffer_option(MTLResourceOptions option);

MTLClearColor hs_rhi_to_clear_color(const float* color);
void          hs_rhi_from_clear_color(MTLClearColor color, float* outColor);

HS_NS_END

#endif
