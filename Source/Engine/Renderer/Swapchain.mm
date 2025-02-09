#include "Engine/Renderer/Swapchain.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Window.h"

#include "Engine/Renderer/CommandHandle.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/Framebuffer.h"
#include "Engine/Renderer/RHIUtility.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

HS_NS_BEGIN

Swapchain::Swapchain(SwapchainInfo info)
    : _drawable(nullptr)
    , _info(info)
{
    _nativeHandle       = static_cast<NativeWindowHandle*>(info.nativeWindowHandle);
    CAMetalLayer* layer = (__bridge CAMetalLayer*)_nativeHandle->layer;

    RenderPassInfo rpInfo{};

    std::vector<Attachment>& attachments = rpInfo.colorAttachments;
    
    attachments.resize(1);
    
    EPixelFormat format = hs_rhi_from_pixel_format(layer.pixelFormat);
//    info.colorFormat = format;
    

    attachments[0].format         = format;
    attachments[0].isDepthStencil = false;
    attachments[0].loadAction     = ELoadAction::CLEAR;
    attachments[0].storeAction    = EStoreAction::STORE;

    attachments[0].clearColor[0] = 0.2f;
    attachments[0].clearColor[1] = 0.2f;
    attachments[0].clearColor[2] = 0.2f;
    attachments[0].clearColor[3] = 1.0f;
    
    if(info.useDepth && info.useStencil)
    {
        //... Depth Buffer Setting
    }
    else if(info.useDepth)
    {
        
    }
    
    if(info.useMSAA)
    {
        //...
    }

    rpInfo.isSwapchainRenderPass     = true;
//    rpInfo.colorAttachments          = attachments;
    rpInfo.colorAttachmentCount      = 1;
    rpInfo.depthStencilAttachment    = {};
    rpInfo.useDepthStencilAttachment = false;
    

    _renderPass = new RenderPass(rpInfo);
    //    _renderPass->handle =
}

Swapchain::~Swapchain()
{
    if (nullptr != _framebuffers)
    {
        for (size_t i = 0; i < _maxFrameCount; i++)
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

    if (nullptr != _renderPass)
    {
        delete _renderPass;
        _renderPass = nullptr;
    }
}

void Swapchain::Submit(CommandBuffer** cmdBuffers, size_t bufferCount)
{
}

HS_NS_END
