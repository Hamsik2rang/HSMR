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
    Swapchain(const SwapchainInfo& desc);
    virtual ~Swapchain();

    void Update(uint32 width, uint32 height);
    
    HS_FORCEINLINE SwapchainInfo GetInfo() { return _info; }
    
protected:
    SwapchainInfo _info;
};

HS_NS_END

#endif /* Swapchian_h */
