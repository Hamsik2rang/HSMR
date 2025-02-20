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

    virtual uint8  GetMaxFrameIndex() const             = 0;
    virtual uint8  GetCurrentFrameIndex() const         = 0;
    virtual CommandBuffer* GetCommandBufferForCurrentFrame()    = 0;
    virtual CommandBuffer* GetCommandBufferByIndex(uint8 index) = 0;

    const SwapchainInfo info;

protected:
    // RHIContext로만 생성할 수 있도록
    Swapchain(const SwapchainInfo& desc);
};

HS_NS_END

#endif /* Swapchian_h */
