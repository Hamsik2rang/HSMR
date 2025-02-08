//
//  Swapchian.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_SWAPCHAIN_H__
#define __HS_SWAPCHAIN_H__

#include "Precompile.h"

#include "Engine/Renderer/RenderDefinition.h"

HS_NS_BEGIN

class Renderer;
class RenderPass;
class Framebuffer;
class CommandBuffer;

class Swapchain
{
    friend Renderer;

public:
    Swapchain(const SwapchainInfo& desc);
    ~Swapchain();

    void Submit(CommandBuffer** cmdBuffers, size_t bufferCount);

    HS_FORCEINLINE RenderPass*   GetRenderPass() { return _renderPass; }
    HS_FORCEINLINE SwapchainInfo GetInfo() { return _info; }

private:
    SwapchainInfo _info;

    RenderPass*  _renderPass;
    Framebuffer** _framebuffers;
    uint32       _submitCount;
    uint32       _submitIndex;
    void*        _layer;
    void*        _drawable;
};

HS_NS_END

#endif /* Swapchian_h */
