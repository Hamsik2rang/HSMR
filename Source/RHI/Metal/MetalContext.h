//
//  RHIContextMetal.h
//  Engine
//
//  Created by Yongsik Im on 2/11/25.
//

#ifndef __HS_RHI_CONTEXT_METAL_H__
#define __HS_RHI_CONTEXT_METAL_H__

#include "Precompile.h"

#include "Core/RHI/RHIContext.h"

HS_NS_BEGIN

class RHIContextMetal : public RHIContext
{
public:
    RHIContextMetal() {}
    ~RHIContextMetal() override {}

    bool Initialize() override;
    void Finalize() override;

    void Suspend(Swapchain* swapchain) override;
    void Restore(Swapchain* swapchain) override;

    uint32 AcquireNextImage(Swapchain* swapchain) override;

    Swapchain* CreateSwapchain(SwapchainInfo info) override;
    void       DestroySwapchain(Swapchain* swapchain) override;

    RenderPass* CreateRenderPass(const char* name, const RenderPassInfo& info) override;
    void        DestroyRenderPass(RenderPass* renderPass) override;

    Framebuffer* CreateFramebuffer(const char* name, const FramebufferInfo& info) override;
    void         DestroyFramebuffer(Framebuffer* framebuffer) override;

    GraphicsPipeline* CreateGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info) override;
    void              DestroyGraphicsPipeline(GraphicsPipeline* pipeline) override;

    Shader* CreateShader(const char* name, const ShaderInfo& info, const char* path) override;
    Shader* CreateShader(const char* name, const ShaderInfo& info, const char* byteCode, size_t byteCodeSize) override;
    void    DestroyShader(Shader* shader) override;

    Buffer* CreateBuffer(const char* name, const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) override;
    Buffer* CreateBuffer(const char* name, const void* data, size_t dataSize, const BufferInfo& info) override;
    void    DestroyBuffer(Buffer* buffer) override;

    Texture* CreateTexture(const char* name, void* image, const TextureInfo& info) override;
    Texture* CreateTexture(const char* name, void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) override;
    void     DestroyTexture(Texture* texture) override;

    Sampler* CreateSampler(const char* name, const SamplerInfo& info) override;
    void     DestroySampler(Sampler* sampler) override;

    ResourceLayout* CreateResourceLayout(const char* name, ResourceBinding* bindings, uint32 bindingCount) override;
    void            DestroyResourceLayout(ResourceLayout* rLayout) override;

    ResourceSet* CreateResourceSet(const char* name, ResourceLayout* resourceLayouts) override;
    void         DestroyResourceSet(ResourceSet* resSet) override;

    ResourceSetPool* CreateResourceSetPool(const char* name, uint32 bufferSize, uint32 textureSize) override;
    void             DestroyResourceSetPool(ResourceSetPool* resSetPool) override;

    CommandPool* CreateCommandPool(const char* name, uint32 queueFamilyIndex = 0) override;
    void         DestroyCommandPool(CommandPool* cmdPool) override;

    CommandBuffer* CreateCommandBuffer(const char* name) override;
    void           DestroyCommandBuffer(CommandBuffer* cmdBuffer) override;

    void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) override;

    void Present(Swapchain* swapchain) override;

    void WaitForIdle() const override;

    HS_FORCEINLINE void* GetDevice() const { return _device; }

private:
    void* _device = nullptr;
};

HS_NS_END

#endif /* __HS_RHI_CONTEXT_METAL_H__ */
