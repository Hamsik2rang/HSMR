//
//  RHIUtility.h
//  Engine
//
//  Created by Yongsik Im on 2/9/25.
//
// CAUTION!: 이 헤더는 .mm파일 외에서는 절대 포함시키지 않습니다.
#ifndef __HS_RHI_UTILITY_H__
#define __HS_RHI_UTILITY_H__

#include "Engine/RHI/RenderDefinition.h"

HS_NS_BEGIN

MTLPixelFormat hs_rhi_to_pixel_format(EPixelFormat format);
EPixelFormat hs_rhi_from_pixel_format(MTLPixelFormat format);

MTLLoadAction hs_rhi_to_load_action(ELoadAction action);
ELoadAction hs_rhi_from_load_action(MTLLoadAction action);

MTLStoreAction hs_rhi_to_store_action(EStoreAction action);
EStoreAction hs_rhi_from_store_action(MTLStoreAction action);

MTLViewport hs_rhi_to_viewport(Viewport vp);
Viewport hs_rhi_from_viewport(MTLViewport vp);

MTLTextureUsage hs_rhi_to_texture_usage(ETextureUsage usage);
ETextureUsage hs_rhi_from_texture_usage(MTLTextureUsage usage);

MTLTextureType hs_rhi_to_texture_type(ETextureType type);
ETextureType hs_rhi_from_texture_type(MTLTextureType type);

CAMetalLayer* hs_rhi_to_layer(void* layer);
void* hs_rhi_from_layer(CAMetalLayer* layer);

id<MTLDevice> hs_rhi_to_device(void* device);
void* hs_rhi_from_device(id<MTLDevice> device);

id<MTLCommandQueue> hs_rhi_to_command_queue(void* q);
void* hs_rhi_from_command_queue(id<MTLCommandQueue> q);

MTLClearColor hs_rhi_to_clear_color(const float* color);
void hs_rhi_from_clear_color(MTLClearColor color, float* outColor);

MTLRenderPassDescriptor* hs_rhi_to_render_pass_desc(void* desc);
void* hs_rhi_from_render_pass_desc(MTLRenderPassDescriptor* desc);

id<MTLTexture> hs_rhi_to_texture(void* texture);
void* hs_rhi_from_texture(id<MTLTexture> texture);

HS_NS_END

#endif
