//
//  RendererPass.m
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//

#include "Engine/RendererPass/RendererPass.h"
#include "Engine/Renderer/Renderer.h"

#import <Metal/Metal.h>


HS_NS_BEGIN

RHI_RESOURCE_BEGIN
    id<MTLDevice> device;
    id<MTLCommandQueue> cmdQueue;
RHI_RESOURCE_END




HS_NS_END
