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

HS_NS_BEGIN

class CommandBuffer;
class RenderTarget;
class NativeWindowHandle;

class Swapchain
{
public:
    virtual ~Swapchain();

    //    HS_FORCEINLINE void SubmitCommandBuffers(CommandBuffer** cmdBuffers, size_t bufferCount);
    //    HS_FORCEINLINE const std::vector<CommandBuffer*>& GetSubmittedCmdBuffers() const {return _submittedCmdBuffers;}
    //    HS_FORCEINLINE void ClearSubmittedCmdBuffers() { _submittedCmdBuffers.clear();}

    virtual uint8          GetMaxFrameIndex() const                   = 0;
    virtual uint8          GetCurrentFrameIndex() const               = 0;
    virtual CommandBuffer* GetCommandBufferForCurrentFrame() const    = 0;
    virtual CommandBuffer* GetCommandBufferByIndex(uint8 index) const = 0;
    virtual RenderTarget   GetRenderTargetForCurrentFrame() const     = 0;

    HS_FORCEINLINE uint32 GetWidth() { return _info.width; }
    HS_FORCEINLINE uint32 GetHeight() { return _info.height; }

    HS_FORCEINLINE void SetWidth(uint32 width) { _info.width = width; }
    HS_FORCEINLINE void SetHeight(uint32 height) { _info.height = height; }

    HS_FORCEINLINE SwapchainInfo GetInfo() const { return _info; }
    HS_FORCEINLINE RenderPass*   GetRenderPass() const { return _renderPass; }

protected:
    virtual void setRenderTargets() = 0;
    virtual void setRenderPass()   = 0;

    SwapchainInfo _info;

    RenderPass*               _renderPass;
    std::vector<RenderTarget> _renderTargets;

    // RHIContext로만 생성할 수 있도록
    Swapchain(const SwapchainInfo& desc);
};

HS_NS_END

#endif /* Swapchian_h */
