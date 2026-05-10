//
//  MetalSwapchain.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_SWAPCHAIN_METAL_H__
#define __HS_SWAPCHAIN_METAL_H__

#include "Precompile.h"

#include "RHI/Swapchain.h"

#include "RHI/Metal/MetalUtility.h"
#include "RHI/ResourceHandle.h"

#import <MetalKit/MetalKit.h>

HS_NS_BEGIN

class CommandBuffer;

class MetalSwapchain : public Swapchain
{
    friend class MetalContext;

public:
    MetalSwapchain(const SwapchainInfo& info);
    ~MetalSwapchain() override;

    void* nativeHandle;

    HS_FORCEINLINE uint8 GetMaxFrameCount() const override { return _maxFrameCount; }
    HS_FORCEINLINE uint8 GetCurrentFrameIndex() const override { return _frameIndex; }
    HS_FORCEINLINE uint8 GetCurrentImageIndex() const override { return _imageIndex; }
    HS_FORCEINLINE RHICommandBuffer* GetCommandBufferForCurrentFrame() const override { return _commandBuffers[_frameIndex]; }
    HS_FORCEINLINE RHICommandBuffer* GetCommandBufferByIndex(uint8 index) const override
    {
        HS_ASSERT(index < _maxFrameCount, "Count of commandbuffer is less than index");
        return _commandBuffers[index];
    }
    HS_FORCEINLINE RHITexture* GetCurrentColorTexture() const override { return _colorTextures[_frameIndex]; }
    HS_FORCEINLINE EPixelFormat GetColorFormat() const override
    {
        HS_ASSERT(_colorTextures, "Swapchain Texture isn't initialized yet.");
        return _colorTextures[0]->info.format;
    }

private:
    uint8 _frameIndex;
    uint8 _imageIndex;
    uint8 _maxFrameCount = 3;

    void setRenderTargets();

    id<CAMetalDrawable> _drawable;
    RHICommandBuffer** _commandBuffers;
    RHITexture** _colorTextures;

    bool _isSuspended;
    bool _isInitialized;
};

HS_NS_END

#endif
