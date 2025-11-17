//
//  Swapchian.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_SWAPCHAIN_H__
#define __HS_SWAPCHAIN_H__

#include "Precompile.h"

#include "Platform/NativeWindow.h"
#include "RHI/RHIDefinition.h"

namespace HS { class RHICommandBuffer; }
namespace HS { class RHIFramebuffer; }

HS_NS_BEGIN

class HS_RHI_API Swapchain : public RHIHandle
{
public:
    Swapchain(const SwapchainInfo& info);
    virtual ~Swapchain();

    //    HS_FORCEINLINE void SubmitCommandBuffers(CommandBuffer** cmdBuffers, size_t bufferCount);
    //    HS_FORCEINLINE const std::vector<CommandBuffer*>& GetSubmittedCmdBuffers() const {return _submittedCmdBuffers;}
    //    HS_FORCEINLINE void ClearSubmittedCmdBuffers() { _submittedCmdBuffers.clear();}

    virtual uint8           GetMaxFrameCount() const                   = 0;
    virtual uint8           GetCurrentFrameIndex() const               = 0;
    virtual RHICommandBuffer*  GetCommandBufferForCurrentFrame() const    = 0;
    virtual RHICommandBuffer*  GetCommandBufferByIndex(uint8 index) const = 0;
    virtual RHIFramebuffer*    GetFramebufferForCurrentFrame() const = 0;

    HS_FORCEINLINE uint32 GetWidth() { return _info.nativeWindow->surfaceWidth; }
    HS_FORCEINLINE uint32 GetHeight() { return _info.nativeWindow->surfaceHeight; }

    HS_FORCEINLINE SwapchainInfo GetInfo() const { return _info; }
    HS_FORCEINLINE RHIRenderPass*   GetRenderPass() const { return _renderPass; }

protected:
    SwapchainInfo   _info;
    RHIRenderPass*     _renderPass;
};

HS_NS_END

#endif /* Swapchian_h */
