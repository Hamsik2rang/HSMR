#include "RHI/Metal/MetalSwapchain.h"

#include "Core/Log.h"
#include "Core/Native/NativeWindow.h"

#include "RHI/RHIContext.h"
#include "RHI/RHIDefinition.h"

#include "RHI/Metal/MetalUtility.h"
#include "RHI/Metal/MetalRenderHandle.h"
#include "RHI/Metal/MetalCommandHandle.h"
#include "RHI/Metal/MetalResourceHandle.h"

#import <MetalKit/MetalKit.h>

@class HSViewController;

HS_NS_BEGIN

SwapchainMetal::SwapchainMetal(const SwapchainInfo& info)
    : Swapchain(info)
    , _frameIndex(0)
    , _maxFrameCount(3)
    , _drawable(nil)
{
    const NativeWindow* nh = info.nativeWindow;
    nativeHandle           = nh->handle;

    NSWindow* window     = (__bridge NSWindow*)(nh->handle);
    HSViewController* vc = (HSViewController*)[window delegate];

    // Configure VSync on CAMetalLayer
    NSView* contentView = [window contentView];
    if ([contentView.layer isKindOfClass:[CAMetalLayer class]])
    {
        CAMetalLayer* metalLayer = (CAMetalLayer*)contentView.layer;
        metalLayer.displaySyncEnabled = info.enableVSync ? YES : NO;
    }

    _commandBuffers = new RHICommandBuffer*[_maxFrameCount];

    RHIContext* rhiContext = RHIContext::Get();

    for (uint8 i = 0; i < _maxFrameCount; i++)
    {
        _commandBuffers[i] = rhiContext->CreateCommandBuffer("CommandBuffer in Swapchain");
    }

    setRenderPass();
    setFramebuffers();
}

SwapchainMetal::~SwapchainMetal()
{
    RHIContext* rhiContext = RHIContext::Get();
    
    for (uint8 i = 0; i < _maxFrameCount; i++)
    {
        if (nullptr != _commandBuffers[i])
        {
            rhiContext->DestroyCommandBuffer(_commandBuffers[i]);
            _commandBuffers[i] = nullptr;
        }
        
        if(nullptr != _framebuffers[i])
        {
            rhiContext->DestroyFramebuffer(_framebuffers[i]);
            _framebuffers[i] = nullptr;
        }
    }
    delete[] _commandBuffers;
    delete[] _framebuffers;
}

void SwapchainMetal::setRenderPass()
{
    RenderPassInfo info{};
    info.isSwapchainRenderPass = true;
    info.colorAttachmentCount  = 1;
    Attachment colorAttachment{};
    colorAttachment.format         = EPixelFormat::B8G8A8R8_UNORM;
    colorAttachment.clearValue     = ClearValue(0.5, 0.5, 0.5, 1.0);
    colorAttachment.loadAction     = ELoadAction::CLEAR;
    colorAttachment.storeAction    = EStoreAction::STORE;
    colorAttachment.isDepthStencil = false;
    info.colorAttachments.push_back(colorAttachment);

    _renderPass = RHIContext::Get()->CreateRenderPass("RenderPass in Swapchain", info);
}

void SwapchainMetal::setFramebuffers()
{
    HS_ASSERT(_framebuffers == nullptr, "Framebuffer is already exists. you should destroy it before creating new one.");

    RHIContext* rhiContext = RHIContext::Get();
    _framebuffers          = new RHIFramebuffer*[_maxFrameCount];

    for (uint8 i = 0; i < _maxFrameCount; i++)
    {
        TextureInfo tInfo{};
        tInfo.arrayLength          = 0;
        tInfo.extent.width         = _info.nativeWindow->surfaceWidth;
        tInfo.extent.height        = _info.nativeWindow->surfaceHeight;
        tInfo.extent.depth         = 1;
        tInfo.format               = EPixelFormat::B8G8A8R8_UNORM;
        tInfo.usage                = ETextureUsage::COLOR_ATTACHMENT;
        tInfo.isCompressed         = false;
        tInfo.useGenerateMipmap    = false;
        tInfo.mipLevel             = 1;
        tInfo.isSwapchainTexture   = true;
        tInfo.type                 = ETextureType::TEX_2D;
        tInfo.swapchain            = this;
        tInfo.byteSize             = tInfo.extent.width * tInfo.extent.height * 4; // Assuming 4 bytes per pixel
        tInfo.isDepthStencilBuffer = false;

        RHITexture* texture = rhiContext->CreateTexture("Swapchain Framebffer Texture", nullptr, tInfo);

        FramebufferInfo fbInfo{};
        fbInfo.depthStencilBuffer     = nullptr;
        fbInfo.resolveBuffer          = nullptr;
        fbInfo.isSwapchainFramebuffer = true;
        fbInfo.width                  = tInfo.extent.width;
        fbInfo.height                 = tInfo.extent.height;
        fbInfo.renderPass             = _renderPass;
        fbInfo.colorBuffers.push_back(texture);

        RHIFramebuffer* framebuffer = rhiContext->CreateFramebuffer("Swapchain Framebuffer", fbInfo);
        _framebuffers[i]            = framebuffer;
    }
}

HS_NS_END
