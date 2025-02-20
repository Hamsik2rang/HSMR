//
//  SwapchainMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/14/25.
//
#ifndef __HS_SWAPCHAIN_METAL_H__
#define __HS_SWAPCHAIN_METAL_H__

#include "Precompile.h"

#include "Engine/Core/Swapchain.h"

#include "Engine/RHI/Metal/RHIUtilityMetal.h"

HS_NS_BEGIN

class CommandBuffer;

class SwapchainMetal : public Swapchain
{
public:
    SwapchainMetal(const SwapchainInfo& info);
    ~SwapchainMetal() override;

    CAMetalLayer*       layer;
    id<CAMetalDrawable> drawable;
    MTKView*            view;

    void* nativeHandle;

    uint8          GetMaxFrameIndex() const override { return _maxFrameIndex; }
    uint8          GetCurrentFrameIndex() const override { return frameIndex; }
    CommandBuffer* GetCommandBufferForCurrentFrame() override { return _commandBuffers[frameIndex]; }
    CommandBuffer* GetCommandBufferByIndex(uint8 index) override
    {
        HS_ASSERT(index < _commandBuffers.size(), "Count of commandbuffer is less than index");
        return _commandBuffers[index];
    }

    uint8                       frameIndex;
protected:
    uint8                       _maxFrameIndex = 3;
    std::vector<CommandBuffer*> _commandBuffers;
};

HS_NS_END

#endif
