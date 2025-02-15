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

HS_NS_BEGIN

class RHIContextMetal : public RHIContext
{
    bool Initialize() override;
    void Finalize() override;

    uint32 NextFrame(Swapchain* swapchain) override;

    Swapchain* CreateSwapchain(SwapchainInfo info) override;
    void       DestroySwapchain(Swapchain* swapchain) override;

    RenderPass* CreateRenderPass(const RenderPassInfo& info) override;
    void        DestroyRenderPass(RenderPass* renderPass) override;

    Framebuffer* CreateFramebuffer(const FramebufferInfo& info) override;
    void         DestroyFramebuffer(Framebuffer* framebuffer) override;

    GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineInfo& info) override;
    void              DestroyGraphicsPipeline(GraphicsPipeline* pipeline) override;

    Shader* CreateShader(EShaderStage stage, const char* path, const char* entryName, bool isBuiltIn = true) override;
    Shader* CreateShader(EShaderStage stage, const char* byteCode, size_t byteCodeSize, const char* entryName, bool isBuitIn = true) override;
    void    DestroyShader(Shader* shader) override;

    Buffer* CreateBuffer() override;
    void    DestroyBuffer(Buffer* buffer) override;

    Texture* CreateTextrue(void* image, const TextureInfo& info) override;
    Texture* CreateTexture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) override;
    void     DestroyTexture(Texture* texture) override;

    ResourceLayout CreateResourceLayout() override;
    void           DestroyResourceLayout(ResourceLayout* resourceLayout) override;

    DescriptorSet* CreateDescriptorSet() override;
    void           DestroyDescriptorSet(DescriptorSet* dSet) override;

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
};

HS_NS_END

#endif /* __RHI_CONTEXT_METAL_H__ */
