//
//  Swapchian.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_SWAPCHAIN_H__
#define __HS_SWAPCHAIN_H__

#include "Precompile.h"

#include "Engine/RHI/RHIDefinition.h"
#include "Engine/Core/Log.h"
#include "Engine/Renderer/RenderTarget.h"

#include "Engine/Platform/PlatformWindow.h"

namespace HS { class CommandBuffer; }
namespace HS { class RenderTarget; }
namespace HS { class Framebuffer; }

HS_NS_BEGIN

class Swapchain
{
public:
    Swapchain(const SwapchainInfo& info);
    virtual ~Swapchain();

    //    HS_FORCEINLINE void SubmitCommandBuffers(CommandBuffer** cmdBuffers, size_t bufferCount);
    //    HS_FORCEINLINE const std::vector<CommandBuffer*>& GetSubmittedCmdBuffers() const {return _submittedCmdBuffers;}
    //    HS_FORCEINLINE void ClearSubmittedCmdBuffers() { _submittedCmdBuffers.clear();}

    virtual uint8          GetMaxFrameCount() const                   = 0;
    virtual uint8          GetCurrentFrameIndex() const               = 0;
    virtual CommandBuffer* GetCommandBufferForCurrentFrame() const    = 0;
    virtual CommandBuffer* GetCommandBufferByIndex(uint8 index) const = 0;
    virtual RenderTarget   GetRenderTargetForCurrentFrame() const     = 0;
    virtual Framebuffer* GetFramebufferForCurrentFrame() const = 0;

    HS_FORCEINLINE uint32 GetWidth() { return _info.nativeWindow->surfaceWidth; }
    HS_FORCEINLINE uint32 GetHeight() { return _info.nativeWindow->surfaceHeight; }

    HS_FORCEINLINE SwapchainInfo GetInfo() const { return _info; }
    HS_FORCEINLINE RenderPass*   GetRenderPass() const { return _renderPass; }

protected:
    virtual void setRenderTargets() = 0;
    virtual void setRenderPass()    = 0;

    SwapchainInfo _info;

    RenderPass*                 _renderPass;
    std::vector<RenderTarget>   _renderTargets;
};

HS_NS_END

#endif /* Swapchian_h */
