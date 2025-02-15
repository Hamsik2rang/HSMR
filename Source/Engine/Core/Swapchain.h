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

HS_NS_BEGIN

class CommandBuffer;
class RenderTarget;
class NativeWindowHandle;

class Swapchain
{
public:
    Swapchain(SwapchainInfo desc);
    ~Swapchain();

    void Submit(CommandBuffer** cmdBuffers, size_t bufferCount);

    void Update(uint32 width, uint32 height);
    
    HS_FORCEINLINE SwapchainInfo GetInfo() { return _info; }
    
    HS_FORCEINLINE RenderTarget* GetRenderTarget() { return _renderTarget; }
    HS_FORCEINLINE void* GetNativeWindowHandle() { return _nativeHandle; }
  
    HS_FORCEINLINE uint32 GetCurrentFrameIndex() const { return _frameIndex; }
    
private:
    uint32 _frameIndex;
    SwapchainInfo _info;
    
    NativeWindowHandle* _nativeHandle;
};

HS_NS_END

#endif /* Swapchian_h */
