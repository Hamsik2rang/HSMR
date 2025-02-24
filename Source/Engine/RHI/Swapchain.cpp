#include "Engine/RHI/Swapchain.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Window.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

Swapchain::Swapchain(const SwapchainInfo& info)
    : _info(info)
    , _width(1)
    , _height(1)
{
}

Swapchain::~Swapchain()
{
    hs_engine_get_rhi_context()->WaitForIdle();
    if (nullptr != _renderPass)
    {
        delete _renderPass;
        _renderPass = nullptr;
    }
    
    _renderTargets.clear();
    
    
}

HS_NS_END
