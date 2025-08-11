#include "Core/Swapchain.h"

#include "Core/Log.h"
#include "Core/Window.h"

HS_NS_BEGIN

Swapchain::Swapchain(const SwapchainInfo& info)
    : _info(info)
    , _renderPass(nullptr)
{
}

Swapchain::~Swapchain()
{
    hs_engine_get_rhi_context()->WaitForIdle();

    _renderTargets.clear();
}

HS_NS_END
