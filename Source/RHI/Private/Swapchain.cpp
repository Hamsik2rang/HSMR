#include "RHI/Swapchain.h"

#include "Core/Log.h"
#include "Core/Window.h"

HS_NS_BEGIN

Swapchain::Swapchain(const SwapchainInfo& info)
	: RHIHandle(RHIHandle::EType::SWAPCHAIN, info.nativeWindow->title)
    , _info(info)
    , _renderPass(nullptr)
{
}

Swapchain::~Swapchain()
{

}

HS_NS_END
