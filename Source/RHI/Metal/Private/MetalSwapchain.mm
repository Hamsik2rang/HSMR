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

HS_NS_BEGIN

MetalSwapchain::MetalSwapchain(const SwapchainInfo& info)
    : Swapchain(info)
    , _frameIndex(0)
    , _imageIndex(0)
    , _maxFrameCount(3)
    , _drawable(nil)
    , _colorTextures(nullptr)
{
    const NativeWindow* nh = info.nativeWindow;
    nativeHandle           = nh->handle;

    _commandBuffers = new RHICommandBuffer*[_maxFrameCount];

    RHIContext* rhiContext = RHIContext::Get();

    for (uint8 i = 0; i < _maxFrameCount; i++)
    {
        _commandBuffers[i] = rhiContext->CreateCommandBuffer("CommandBuffer in Swapchain");
    }

    setRenderTargets();
}

MetalSwapchain::~MetalSwapchain()
{
    RHIContext* rhiContext = RHIContext::Get();
    
    for (uint8 i = 0; i < _maxFrameCount; i++)
    {
        if (nullptr != _commandBuffers[i])
        {
            rhiContext->DestroyCommandBuffer(_commandBuffers[i]);
            _commandBuffers[i] = nullptr;
        }
        
        if(nullptr != _colorTextures[i])
        {
            rhiContext->DestroyTexture(_colorTextures[i]);
            _colorTextures[i] = nullptr;
        }
    }
    delete[] _commandBuffers;
    delete[] _colorTextures;
}

void MetalSwapchain::setRenderTargets()
{
    HS_ASSERT(_colorTextures == nullptr, "Swapchain render targets already exist. Destroy them before creating new ones.");

    _colorTextures         = new RHITexture*[_maxFrameCount]{nullptr};

    for (uint8 i = 0; i < _maxFrameCount; i++)
    {
        TextureInfo tInfo{};
        tInfo.arrayLength          = 1;
        tInfo.extent.width         = _info.nativeWindow->surfaceWidth;
        tInfo.extent.height        = _info.nativeWindow->surfaceHeight;
        tInfo.extent.depth         = 1;
        tInfo.format               = EPixelFormat::B8G8A8R8Unorm;
        tInfo.usage                = ETextureUsage::ColorAttachment;
        tInfo.isCompressed         = false;
        tInfo.useGenerateMipmap    = false;
        tInfo.mipLevel             = 1;
        tInfo.isSwapchainTexture   = true;
        tInfo.type                 = ETextureType::Tex2D;
        tInfo.swapchain            = this;
        tInfo.byteSize             = tInfo.extent.width * tInfo.extent.height * 4; // Assuming 4 bytes per pixel
        tInfo.isDepthStencilBuffer = false;

        _colorTextures[i] = new MetalTexture("Swapchain Texture", tInfo);
    }
}

HS_NS_END
