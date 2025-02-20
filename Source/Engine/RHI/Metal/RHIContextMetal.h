//
//  RHIContextMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/11/25.
//

#ifndef __HS_RHI_CONTEXT_METAL_H__
#define __HS_RHI_CONTEXT_METAL_H__

#include "Precompile.h"

#include "Engine/RHI/RHIContext.h"

HS_NS_BEGIN

class RHIContextMetal : public RHIContext
{
public:
    RHIContextMetal() {}
    ~RHIContextMetal() override {}

    bool Initialize() override;
    void Finalize() override;

    uint32 AcquireNextImage(Swapchain* swapchain) override;

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

    Buffer* CreateBuffer(void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) override;
    Buffer* CreateBuffer(void* data, size_t dataSize, const BufferInfo& info) override;
    void    DestroyBuffer(Buffer* buffer) override;

    Texture* CreateTexture(void* image, const TextureInfo& info) override;
    Texture* CreateTexture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) override;
    void     DestroyTexture(Texture* texture) override;

    Sampler* CreateSampler(const SamplerInfo& info) override;
    void     DestroySampler(Sampler* sampler) override;

    ResourceLayout* CreateResourceLayout() override;
    void            DestroyResourceLayout(ResourceLayout* rLayout) override;

    ResourceSet* CreateResourceSet() override;
    void         DestroyResourceSet(ResourceSet* resSet) override;

    ResourceSetPool* CreateResourceSetPool() override;
    void             DestroyResourceSetPool(ResourceSetPool* resSetPool) override;
    //
    //     CommandQueue* CreateCommandQueue() override;
    //     void DestroyCommandQueue(CommandQueue* cmdQueue) override;

    CommandPool* CreateCommandPool() override;
    void         DestroyCommandPool(CommandPool* cmdPool) override;

    CommandBuffer* CreateCommandBuffer() override;
    void           DestroyCommandBuffer(CommandBuffer* cmdBuffer) override;

    Fence* CreateFence() override;
    void   DestroyFence(Fence* fence) override;

    Semaphore* CreateSemaphore() override;
    void       DestroySemaphore(Semaphore* semaphore) override;

    void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) override;

    void Present(Swapchain* swapchain) override;

    HS_FORCEINLINE void* GetDevice() const { return _device; }

private:
    void* _device = nullptr;
};

HS_NS_END

#endif /* __HS_RHI_CONTEXT_METAL_H__ */
