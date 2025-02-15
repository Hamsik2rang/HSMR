#include "Engine/Renderer/Framebuffer.h"

#include "Engine/Core/Log.h"
#include "Engine/Renderer/RenderPass.h"
#include "Engine/Renderer/ResourceHandle.h"

HS_NS_BEGIN

Framebuffer::Framebuffer(FramebufferInfo info)
    : info(info)
    , _renderPass(info.renderPass)
{
    const RenderPass* rp = info.renderPass;
    HS_ASSERT(info.colorBuffers.size() == rp->info.colorAttachmentCount, "Color buffer size is not matched");
    HS_ASSERT(!(info.depthStencilBuffer == nullptr && rp->info.useDepthStencilAttachment), "depth stencil buffer is invalid");

    //...
}

void Framebuffer::Resize(uint32 width, uint32 height)
{
    for (size_t i = 0; i < info.colorBuffers.size(); i++)
    {
        auto&       colorBuffer = info.colorBuffers[i];
        TextureInfo tInfo       = colorBuffer->texInfo;
        SamplerInfo sInfo       = colorBuffer->smpInfo;
        tInfo.extent.width      = width;
        tInfo.extent.height     = height;

        delete colorBuffer;
        info.colorBuffers[i] = new Texture(nullptr, tInfo, sInfo);
    }

    if (nullptr != info.depthStencilBuffer)
    {
        TextureInfo tInfo = info.depthStencilBuffer->texInfo;
        SamplerInfo sInfo = info.depthStencilBuffer->smpInfo;
        tInfo.extent.width = width;
        tInfo.extent.height = height;
        
        delete info.depthStencilBuffer;
        info.depthStencilBuffer = new Texture(nullptr, tInfo, sInfo);
    }
}

Framebuffer::~Framebuffer()
{
    for (size_t i = 0; i < info.colorBuffers.size(); i++)
    {
        if (nullptr != info.colorBuffers[i])
        {
            delete info.colorBuffers[i];
            info.colorBuffers[i] = nullptr;
        }
    }

    if (nullptr != info.depthStencilBuffer)
    {
        delete info.depthStencilBuffer;
        info.depthStencilBuffer = nullptr;
    }

    if (nullptr != info.resolveBuffer)
    {
        delete info.resolveBuffer;
        info.resolveBuffer = nullptr;
    }
}

HS_NS_END
