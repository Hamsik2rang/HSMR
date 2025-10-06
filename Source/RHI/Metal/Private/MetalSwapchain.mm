#include "RHI/Metal/MetalSwapchain.h"

#include "Core/Log.h"
#include "HAL/NativeWindow.h"

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
    nativeHandle = nh->handle;

    NSWindow*         window = (__bridge NSWindow*)(nh->handle);
    HSViewController* vc     = (HSViewController*)[window delegate];

    _commandBufferMTLs = new RHICommandBuffer*[_maxFrameCount];

    RHIContext* rhiContext = RHIContext::Get();

    for (uint8 i = 0; i < _maxFrameCount; i++)
    {
        _commandBufferMTLs[i] = rhiContext->CreateCommandBuffer("CommandBuffer in Swapchain");
    }

    setRenderPass();
}

SwapchainMetal::~SwapchainMetal()
{
    for (uint8 i = 0; i < _maxFrameCount; i++)
    {
        auto* cmdBuffer = _commandBufferMTLs[i];
        if (nullptr == cmdBuffer)
        {
            delete cmdBuffer;
            cmdBuffer = nullptr;
        }
    }
    delete[] _commandBufferMTLs;
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

HS_NS_END
