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
EPixelFormat hs_rhi_from_pixel_format(MTLPixelFormat format);

MTLLoadAction hs_rhi_to_load_action(ELoadAction action);
ELoadAction hs_rhi_from_laod_action(MTLLoadAction action);

MTLStoreAction hs_rhi_to_store_action(EStoreAction action);
EStoreAction hs_rhi_from_store_action(MTLStoreAction action);

MTLViewport hs_rhi_to_viewport(Viewport vp);
Viewport hs_rhi_from_viewport(MTLViewport vp);

MTLTextureUsage hs_rhi_to_texture_usage(ETextureUsage usage);
ETextureUsage hs_rhi_from_texture_usage(MTLTextureUsage usage);

MTLTextureType hs_rhi_to_texture_type(ETextureType type);
ETextureType hs_rhi_from_texture_type(MTLTextureType type);

MTLClearColor hs_rhi_to_clear_color(const float* color);
void hs_rhi_from_clear_color(MTLClearColor color, float* outColor);

HS_NS_END

#endif
