#include "Engine/RHI/Metal/SwapchainMetal.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Window.h"

#include "Engine/Platform/Mac/PlatformWindowMac.h"

#include "Engine/Renderer/RenderDefinition.h"

#include "Engine/RHI/Metal/RHIUtilityMetal.h"
#include "Engine/RHI/Metal/RenderHandleMetal.h"
#include "Engine/RHI/Metal/CommandHandleMetal.h"
#include "Engine/RHI/Metal/ResourceHandleMetal.h"

#import <MetalKit/MetalKit.h>

HS_NS_BEGIN

SwapchainMetal::SwapchainMetal(const SwapchainInfo& info)
    : Swapchain(info)
    , frameIndex(0)
    , maxFrameCount(3)
    , _drawable(nil)
{
    const NativeWindow* nh = info.nativeWindow;

    NSWindow*         window = (__bridge_transfer NSWindow*)(nh->handle);
    HSViewController* vc     = (HSViewController*)[window delegate];

    commandBufferMTLs = new CommandBuffer*[maxFrameCount];
    _renderTargets.resize(maxFrameCount);

    RHIContext* rhiContext = hs_engine_get_rhi_context();

    for (uint8 i = 0; i < maxFrameCount; i++)
    {
        commandBufferMTLs[i] = rhiContext->CreateCommandBuffer();
    }

    setRenderTargets();
    setRenderPass();
}

SwapchainMetal::~SwapchainMetal()
{
    for (uint8 i = 0; i < maxFrameCount; i++)
    {
        auto* cmdBuffer = commandBufferMTLs[i];
        if (nullptr == cmdBuffer)
        {
            delete cmdBuffer;
            cmdBuffer = nullptr;
        }
    }
    delete[] commandBufferMTLs;
}

void SwapchainMetal::setRenderTargets()
{
    RenderTargetInfo info{};
    info.isSwapchainTarget      = true;
    info.colorTextureCount      = 1;
    info.useDepthStencilTexture = false; // TOOD: 선택 가능하면 좋을듯함.

    TextureInfo colorTextureInfo{};
    colorTextureInfo.extent.width         = (_info.nativeWindow)->width;
    colorTextureInfo.extent.height        = (_info.nativeWindow)->height;
    colorTextureInfo.extent.depth         = 1;
    colorTextureInfo.format               = EPixelFormat::B8G8A8R8_UNORM;
    colorTextureInfo.usage                = ETextureUsage::RENDER_TARGET;
    colorTextureInfo.arrayLength          = 1;
    colorTextureInfo.mipLevel             = 1;
    colorTextureInfo.isCompressed         = false;
    colorTextureInfo.useGenerateMipmap    = false;
    colorTextureInfo.isDepthStencilBuffer = false;
    colorTextureInfo.type                 = ETextureType::TEX_2D;

    info.colorTextureInfos.emplace_back(colorTextureInfo);

    for (auto& renderTarget : _renderTargets)
    {
        renderTarget.Create(info);
    }
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

    _renderPass = hs_engine_get_rhi_context()->CreateRenderPass(info);
}

HS_NS_END
