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
{
    NativeWindowHandle* nativeHandle = reinterpret_cast<NativeWindowHandle*>(info.nativeWindowHandle);
    view = (__bridge_transfer MTKView*)nativeHandle->view;
    drawable = nil;
    layer = (__bridge_transfer CAMetalLayer*)(nativeHandle->layer);
    
    _maxFrameIndex = layer.maximumDrawableCount;
    
}

SwapchainMetal::~SwapchainMetal()
{
    
}



HS_NS_END
