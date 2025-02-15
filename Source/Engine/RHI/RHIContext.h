//
//  RHIContext.h
//  Engine
//
//  Created by Yongsik Im on 2/11/25.
//
#ifndef __RHI_CONTEXT_H__
#define __RHI_CONTEXT_H__

#include "Precompile.h"

#include "Engine/RHI/RHIDefinition.h"
#include "Engine/RHI/RenderPass.h"
#include "Engine/RHI/Framebuffer.h"
#include "Engine/RHI/Pipeline.h"
#include "Engine/RHI/CommandHandle.h"
#include "Engine/RHI/ResourceHandle.h"

HS_NS_BEGIN

class RHIContext
{
public:
    virtual bool Initialize() = 0;
    virtual void Finalize()   = 0;

    virtual uint32 NextFrame(Swapchain* swapchain) = 0;

    virtual Swapchain* CreateSwapchain(SwapchainInfo info)    = 0;
    virtual void       DestroySwapchain(Swapchain* swapchain) = 0;

    virtual RenderPass* CreateRenderPass()                        = 0;
    virtual void        DestroyRenderPass(RenderPass* renderPass) = 0;
    
    virtual Framebuffer* CreateFramebuffer() = 0;
    virtual void DestroyFramebuffer(Framebuffer* framebuffer) = 0;

    virtual Pipeline* CreatePipeline() =0;
    virtual void DestroyPipeline() =0;
    
    virtual Shader* CreateShader() = 0;
    virtual void DestroyShader(Shader* shader);
    
    virtual Buffer* CreateBuffer()                = 0;
    virtual void    DestroyBuffer(Buffer* buffer) = 0;

    virtual Texture* CreateTextrue()                  = 0;
    virtual void     DestroyTexture(Texture* texture) = 0;
    
    virtual ResourceLayout CreateResourceLayout() =0;
    virtual void DestroyResourceLayout(ResourceLayout* resourceLayout) = 0;
    
    virtual DescriptorSet* CreateDescriptorSet() =0;
    virtual void DestroyDescriptorSet(DescriptorSet* dSet);
    
    virtual DescriptorPool* CreateDescriptorPool() =0;
    virtual void DestroyDescriptorPool(DescriptorPool* dPool) = 0;
//    
//    virtual CommandQueue* CreateCommandQueue() =0;
//    virtual void DestroyCommandQueue(CommandQueue* cmdQueue) = 0;
    
    virtual CommandPool* CreateCommandPool() =0;
    virtual void DestroyCommandPool(CommandPool* cmdPool) = 0;
    
    virtual CommandBuffer* CreateCommandBuffer() =0;
    virtual void DestroyCommandBuffer(CommandBuffer* cmdBuffer) = 0;
    
    virtual Fence* CreateFence() = 0;
    virtual void DestroyFence(Fence* fence) = 0;
    
    virtual Semaphore CreateSemaphore() = 0;
    virtual void DestroySemaphore(Semaphore* semaphore)  = 0;


    virtual void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) = 0;

    virtual void Present(Swapchain* swapchain) = 0;

private:
};

HS_NS_END

#endif
