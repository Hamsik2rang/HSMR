#include "Engine/Core/Swapchain.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Window.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

Swapchain::Swapchain(const SwapchainInfo& info)
    : info(info)
{

}

Swapchain::~Swapchain()
{

}

// void Swapchain::SubmitCommandBuffers(CommandBuffer** cmdBuffers, size_t bufferCount)
//{
//     size_t submittedBufferCount = _submittedCmdBuffers.size();
//     _submittedCmdBuffers.resize(submittedBufferCount + bufferCount);
//     for (size_t i = 0; i < bufferCount; i++)
//     {
//         _submittedCmdBuffers[submittedBufferCount + i] = cmdBuffers[i];
//     }
// }

HS_NS_END
