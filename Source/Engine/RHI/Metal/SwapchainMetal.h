//
//  SwapchainMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_SWAPCHAIN_METAL_H__
#define __HS_SWAPCHAIN_METAL_H__

#include "Precompile.h"

#include "Engine/RHI/Swapchain.h"

#include "Engine/RHI/Metal/RHIUtilityMetal.h"

HS_NS_BEGIN

class CommandBuffer;

class SwapchainMetal : public Swapchain
{
    friend class RHIContextMetal;
private:
    SwapchainMetal(const SwapchainInfo& info);
    ~SwapchainMetal() override;

    void setRenderTargets() override;
    void setRenderPass() override;
    

    CAMetalLayer*       layer;
    id<CAMetalDrawable> drawable;
    void*               view;
    void* nativeHandle;

    uint8 frameIndex;
    uint8 maxFrameCount = 3;

    CommandBufferMetal** commandBuffers;
};

HS_NS_END

#endif
