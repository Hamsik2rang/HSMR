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

class SwapchainMetal : public Swapchain
{
public:
    SwapchainMetal(const SwapchainInfo& info);
    ~SwapchainMetal() override;

    CAMetalLayer* layer;
    id<CAMetalDrawable> drawable;
    MTKView*      view;

    void*  nativeHandle;
    uint32 frameIndex = 0;

    HS_FORCEINLINE const uint32 GetMaxFrameIndex() const { return _maxFrameIndex; }
private:
    uint32 _maxFrameIndex = 3;
};

HS_NS_END

#endif
