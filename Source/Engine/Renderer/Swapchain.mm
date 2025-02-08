#include "Engine/Renderer/Swapchain.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Window.h"

#include "Engine/Renderer/CommandHandle.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/Framebuffer.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

HS_NS_BEGIN

Swapchain::Swapchain(const SwapchainInfo& info)
    : _drawable(nullptr)
    , _info(info)
{
    _layer              = info.surface;
    CAMetalLayer* layer = (__bridge CAMetalLayer*)_layer;

    RenderPassInfo rpInfo{};

    std::vector<Attachment> attachments(1);

    attachments[0].format         = EPixelFormat::B8G8A8R8_UNORM;
    attachments[0].isDepthStencil = false;
    attachments[0].loadAction     = ELoadAction::CLEAR;
    attachments[0].storeAction    = EStoreAction::STORE;

    rpInfo.isSwapchainRenderPass     = true;
    rpInfo.colorAttachments          = attachments;
    rpInfo.colorAttachmentCount      = 1;
    rpInfo.depthStencilAttachment    = {};
    rpInfo.useDepthStencilAttachment = false;

    _renderPass = new RenderPass(rpInfo);
    
    
}

Swapchain::~Swapchain()
{
    if (nullptr != _framebuffers)
    {
        for (size_t i = 0; i < _submitCount; i++)
        {
            if (nullptr != _framebuffers[i])
            {
                delete _framebuffers[i];
                _framebuffers[i] = nullptr;
            }
        }

        delete[] _framebuffers;
        _framebuffers = nullptr;
    }
    
    if(nullptr != _renderPass)
    {
        delete _renderPass;
        _renderPass = nullptr;
    }
}

void Swapchain::Submit(CommandBuffer** cmdBuffers, size_t bufferCount)
{
}

HS_NS_END
