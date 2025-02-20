#include "Engine/RHI/Metal/SwapchainMetal.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Window.h"

#include "Engine/RHI/Metal/RHIUtilityMetal.h"
#include "Engine/RHI/Metal/RenderHandleMetal.h"
#include "Engine/RHI/Metal/CommandHandleMetal.h"
#include "Engine/RHI/Metal/ResourceHandleMetal.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

SwapchainMetal::SwapchainMetal(const SwapchainInfo& info)
    : Swapchain(info)
    , frameIndex(0)
    , _maxFrameIndex(3)
{
    NativeWindowHandle* nh = reinterpret_cast<NativeWindowHandle*>(info.nativeWindowHandle);

    nativeHandle = nh;
    view         = (__bridge_transfer MTKView*)nh->view;
    drawable     = nil;
    layer        = (__bridge_transfer CAMetalLayer*)(nh->layer);

    _maxFrameIndex = layer.maximumDrawableCount;

    _commandBuffers.resize(_maxFrameIndex);

    RHIContext* rhiContext = hs_engine_get_rhi_context();

    for (uint8 i = 0; i < _maxFrameIndex; i++)
    {
        _commandBuffers[i] = rhiContext->CreateCommandBuffer();
    }
}

SwapchainMetal::~SwapchainMetal()
{
    for (auto& cmdBuffer : _commandBuffers)
    {
        if (nullptr == cmdBuffer)
        {
            delete cmdBuffer;
            cmdBuffer = nullptr;
        }
    }
    _commandBuffers.clear();
}

HS_NS_END
