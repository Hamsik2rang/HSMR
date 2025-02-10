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
class NativeWindowHandle;

class Swapchain
{
    friend Renderer;

public:
    Swapchain(SwapchainInfo desc);
    ~Swapchain();

    void Submit(CommandBuffer** cmdBuffers, size_t bufferCount);

    HS_FORCEINLINE RenderPass*   GetRenderPass() { return _renderPass; }
    HS_FORCEINLINE void          SetRenderPass(RenderPass* renderPass) { _renderPass = renderPass; }
    HS_FORCEINLINE SwapchainInfo GetInfo() { return _info; }
    HS_FORCEINLINE void*         GetDrawable() { return _drawable; }
    HS_FORCEINLINE void          SetDrawable(void* drawable) { return _drawable; }

    void Update(uint32 width, uint32 height);
   
    HS_FORCEINLINE void* GetNativeWindowHandle() { return _nativeHandle; }
    HS_FORCEINLINE uint32 GetWidth() const { return _width; }
    HS_FORCEINLINE uint32 GetHeight() const { return _height; }
    HS_FORCEINLINE uint32 GetCurrentFrameCount() const { return _frameCount;}
    
private:
    SwapchainInfo _info;

    RenderPass*  _renderPass;
    Framebuffer* _framebuffers[3]; //TODO: 매직 넘버 제거하기.
    uint32       _frameCount;
    uint32       _maxFrameCount;

    uint32 _width;
    uint32 _height;

    NativeWindowHandle* _nativeHandle;

    void* _drawable;
};

HS_NS_END

#endif /* Swapchian_h */
