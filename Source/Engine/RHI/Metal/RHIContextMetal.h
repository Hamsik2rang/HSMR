//
//  RHIContextMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/11/25.
//

#ifndef __RHI_CONTEXT_METAL_H__
#define __RHI_CONTEXT_METAL_H__

#include "Precompile.h"

#include "Engine/RHI/RHIContext.h"

#include <Metal/Metal.hpp>
#include <Foundation/Foundation.hpp>
#include <QUartzCore/QuartzCore.hpp>

class RHIContextMetal : public RHIContext
{
    bool Initialize() override;
    void Finalize() override;

    uint32 NextFrame(Swapchain* swapchain) override;

    Swapchain* CreateSwapchain(SwapchainInfo info) override;
    void       DestroySwapchain(Swapchain* swapchain) override;

    RenderPass* CreateRenderPass() override;
    void        DestroyRenderPass(RenderPass* renderPass) override;

    Framebuffer* CreateFramebuffer() override;
    void         DestroyFramebuffer(Framebuffer* framebuffer) override;

    Pipeline* CreatePipeline() override;
    void      DestroyPipeline() override;

    Shader* CreateShader() override;
    void    DestroyShader(Shader* shader);

    Buffer* CreateBuffer() override;
    void    DestroyBuffer(Buffer* buffer) override;

    Texture* CreateTextrue() override;
    void     DestroyTexture(Texture* texture) override;

    ResourceLayout CreateResourceLayout() override;
    void           DestroyResourceLayout(ResourceLayout* resourceLayout) override;

    DescriptorSet* CreateDescriptorSet() override;
    void           DestroyDescriptorSet(DescriptorSet* dSet);

    DescriptorPool* CreateDescriptorPool() override;
    void            DestroyDescriptorPool(DescriptorPool* dPool) override;
    //
    //     CommandQueue* CreateCommandQueue() override;
    //     void DestroyCommandQueue(CommandQueue* cmdQueue) override;

    CommandPool* CreateCommandPool() override;
    void         DestroyCommandPool(CommandPool* cmdPool) override;

    CommandBuffer* CreateCommandBuffer() override;
    void           DestroyCommandBuffer(CommandBuffer* cmdBuffer) override;

    Fence* CreateFence() override;
    void   DestroyFence(Fence* fence) override;

    Semaphore CreateSemaphore() override;
    void      DestroySemaphore(Semaphore* semaphore) override;

    void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) override;

    void Present(Swapchain* swapchain) override;
}

#endif /* __RHI_CONTEXT_METAL_H__ */
