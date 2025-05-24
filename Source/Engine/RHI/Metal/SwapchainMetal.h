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

#import <MetalKit/MetalKit.h>

HS_NS_BEGIN

class CommandBuffer;

class SwapchainMetal : public Swapchain
{
    friend class RHIContextMetal;
public:
    SwapchainMetal(const SwapchainInfo& info);
    ~SwapchainMetal() override;

    void* nativeHandle;

    HS_FORCEINLINE uint8          GetMaxFrameCount() const override { return maxFrameCount; }
    HS_FORCEINLINE uint8          GetCurrentFrameIndex() const override { return frameIndex; }
    HS_FORCEINLINE CommandBuffer* GetCommandBufferForCurrentFrame() const override { return _commandBufferMTLs[frameIndex]; }
    HS_FORCEINLINE CommandBuffer* GetCommandBufferByIndex(uint8 index) const override
    {
        HS_ASSERT(index < maxFrameCount, "Count of commandbuffer is less than index");
        return _commandBufferMTLs[index];
    }
    HS_FORCEINLINE RenderTarget GetRenderTargetForCurrentFrame() const override { return _renderTargets[frameIndex]; }
    
    uint8 frameIndex;
    uint8 maxFrameCount = 3;
    
protected:
    void setRenderTargets() override;
    void setRenderPass() override;
    
    id<CAMetalDrawable> _drawable;
    CommandBuffer** _commandBufferMTLs;
};

HS_NS_END

#endif
