#include "Engine/Core/Swapchain.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Window.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

Swapchain::Swapchain(const SwapchainInfo& info)
    : _info(info)
{
    //    CAMetalLayer* layer = (__bridge CAMetalLayer*)_nativeHandle->layer;
    //
    //    RenderPassInfo rpInfo{};
    //
    //    std::vector<Attachment>& attachments = rpInfo.colorAttachments;
    //
    //    attachments.resize(_info.useDepth ? 2 : 1); // TODO: Refactor
    //
    //    EPixelFormat colorFormat = hs_rhi_from_pixel_format(layer.pixelFormat);
    //    //    info.colorFormat = format;
    //
    //    attachments[0].format         = colorFormat;
    //    attachments[0].isDepthStencil = false;
    //    attachments[0].loadAction     = ELoadAction::CLEAR;
    //    attachments[0].storeAction    = EStoreAction::STORE;
    //
    //    attachments[0].clearValue.color[0] = 0.5f;
    //    attachments[0].clearValue.color[1] = 0.5f;
    //    attachments[0].clearValue.color[2] = 0.5f;
    //    attachments[0].clearValue.color[3] = 1.0f;
    //
    //    if (info.useDepth /*&& info.useStencil*/)
    //    {
    //        //... Depth Buffer Setting
    //        attachments[1].format         = EPixelFormat::DEPTH24_STENCIL8;
    //        attachments[1].isDepthStencil = true;
    //        attachments[1].loadAction     = ELoadAction::CLEAR;
    //        attachments[1].storeAction    = EStoreAction::STORE;
    //
    //        attachments[1].clearValue.depth   = 1.0f;
    //        attachments[1].clearValue.stencil = 0.0f;
    //    }
    //    //    else if(info.useDepth)
    //    //    {
    //    //
    //    //    }
    //    //
    //    if (info.useMSAA)
    //    {
    //        //...
    //    }
    //
    //    rpInfo.isSwapchainRenderPass     = true;
    //    rpInfo.colorAttachments          = attachments;
    //    rpInfo.colorAttachmentCount      = 1;
    //    rpInfo.depthStencilAttachment    = info.useDepth ? attachments[1] : Attachment();
    //    rpInfo.useDepthStencilAttachment = _info.useDepth;
    //
    //    _renderPass = new RenderPass(rpInfo);
    //
    //    SDL_Window* window = static_cast<SDL_Window*>(_nativeHandle->window);
    //    int         width, height;
    //    SDL_GetWindowSizeInPixels(window, &width, &height);
    //
    //    for (size_t i = 0; i < 3; i++)
    //    {
    //        FramebufferInfo fbInfo{};
    //        fbInfo.width  = static_cast<uint32>(width);
    //        fbInfo.height = static_cast<uint32>(height);
    //        fbInfo.colorBuffers.resize(rpInfo.colorAttachmentCount);
    //
    //        TextureInfo tInfo{};
    //        SamplerInfo sInfo{};
    //        for (size_t j = 0; j < rpInfo.colorAttachmentCount; j++)
    //        {
    //            tInfo.type                 = ETextureType::TEX_2D;
    //            tInfo.format               = colorFormat;
    //            tInfo.arrayLength          = 1;
    //            tInfo.extent.width         = width;
    //            tInfo.extent.height        = height;
    //            tInfo.extent.depth         = 1;
    //            tInfo.isCompressed         = false;
    //            tInfo.isDepthStencilBuffer = false;
    //            tInfo.isSwapchianTexture   = true;
    //            tInfo.mipLevel             = 1;
    //            tInfo.usage                = ETextureUsage::RENDER_TARGET;
    //            tInfo.useGenerateMipmap    = false;
    //
    //            sInfo.type              = ETextureType::TEX_2D;
    //            sInfo.addressU          = EAddressMode::CLAMP_TO_EDGE;
    //            sInfo.addressV          = EAddressMode::CLAMP_TO_EDGE;
    //            sInfo.magFilter         = EFilterMode::LINEAR;
    //            sInfo.minFilter         = EFilterMode::LINEAR;
    //            sInfo.mipFilter         = EFilterMode::INVALID;
    //            sInfo.isPixelCoordinate = false;
    //
    //            fbInfo.colorBuffers[j] = new Texture(nullptr, tInfo, sInfo);
    //        }
    //
    //        //        if (_info.useDepth && _info.useStencil)
    //        //        {
    //        //            //...
    //        //        }
    //        //        else if(_info.useDepth)
    //        if (_info.useDepth)
    //        {
    //            tInfo.isDepthStencilBuffer = true;
    //            fbInfo.depthStencilBuffer  = new Texture(nullptr, tInfo, sInfo);
    //        }
    //        else
    //        {
    //            fbInfo.depthStencilBuffer = nullptr;
    //        }
    //
    //        if (_info.useMSAA)
    //        {
    //            //...
    //        }
    //
    //        fbInfo.renderPass = _renderPass;
    //
    //        fbInfo.width                  = width;
    //        fbInfo.height                 = height;
    //        fbInfo.isSwapchainFramebuffer = true;
    //
    //        _framebuffers[i] = new Framebuffer(fbInfo);
    //    }
}

Swapchain::~Swapchain()
{
    //    if (nullptr != _framebuffers)
    //    {
    //        for (size_t i = 0; i < 3; i++)
    //        {
    //            if (nullptr != _framebuffers[i])
    //            {
    //                delete _framebuffers[i];
    //                _framebuffers[i] = nullptr;
    //            }
    //        }
    //    }
    //
    //    if (nullptr != _renderPass)
    //    {
    //        delete _renderPass;
    //        _renderPass = nullptr;
    //    }
}

void Swapchain::Update(uint32 width, uint32 height)
{
    //    if (_width != width || height != height)
    //    {
    //        for (size_t i = 0; i < 3; i++)
    //        {
    //            _framebuffers[i]->Resize(width, height);
    //        }
    //    }
}

HS_NS_END
