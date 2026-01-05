//
//  MetalContext.h
//  Engine
//
//  Created by Yongsik Im on 2/11/25.
//

#ifndef __HS_RHI_CONTEXT_METAL_H__
#define __HS_RHI_CONTEXT_METAL_H__

#include "Precompile.h"

#include "RHI/RHIContext.h"

HS_NS_BEGIN

class HS_API MetalContext : public RHIContext
{
public:
    MetalContext() {}
    ~MetalContext() override {}

    bool Initialize() override;
    void Finalize() override;

    void Suspend(Swapchain* swapchain) override;
    void Restore(Swapchain* swapchain) override;

    uint32 AcquireNextImage(Swapchain* swapchain) override;

    Swapchain* CreateSwapchain(SwapchainInfo info) override;
    void       DestroySwapchain(Swapchain* swapchain) override;

    RHIRenderPass* CreateRenderPass(const char* name, const RenderPassInfo& info) override;
    void        DestroyRenderPass(RHIRenderPass* renderPass) override;

    RHIFramebuffer* CreateFramebuffer(const char* name, const FramebufferInfo& info) override;
    void         DestroyFramebuffer(RHIFramebuffer* framebuffer) override;

    RHIGraphicsPipeline* CreateGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info) override;
    void              DestroyGraphicsPipeline(RHIGraphicsPipeline* pipeline) override;

    RHIComputePipeline* CreateComputePipeline(const char* name, const ComputePipelineInfo& info) override;
    void             DestroyComputePipeline(RHIComputePipeline* pipeline) override;

    RHIShader* CreateShader(const char* name, const ShaderInfo& info, const char* path) override;
    RHIShader* CreateShader(const char* name, const ShaderInfo& info, const char* byteCode, size_t byteCodeSize) override;
    void    DestroyShader(RHIShader* shader) override;

    RHIBuffer* CreateBuffer(const char* name, const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) override;
    RHIBuffer* CreateBuffer(const char* name, const void* data, size_t dataSize, const BufferInfo& info) override;
    void    DestroyBuffer(RHIBuffer* buffer) override;

    RHITexture* CreateTexture(const char* name, void* image, const TextureInfo& info) override;
    RHITexture* CreateTexture(const char* name, void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) override;
    void     DestroyTexture(RHITexture* texture) override;

    RHISampler* CreateSampler(const char* name, const SamplerInfo& info) override;
    void     DestroySampler(RHISampler* sampler) override;

    RHIResourceLayout* CreateResourceLayout(const char* name, ResourceBinding* bindings, uint32 bindingCount) override;
    void            DestroyResourceLayout(RHIResourceLayout* rLayout) override;

    RHIResourceSet* CreateResourceSet(const char* name, RHIResourceLayout* resourceLayouts) override;
    void         DestroyResourceSet(RHIResourceSet* resSet) override;

    RHIResourceSetPool* CreateResourceSetPool(const char* name, uint32 bufferSize, uint32 textureSize) override;
    void             DestroyResourceSetPool(RHIResourceSetPool* resSetPool) override;

    RHICommandPool* CreateCommandPool(const char* name, uint32 queueFamilyIndex = 0) override;
    void         DestroyCommandPool(RHICommandPool* cmdPool) override;

    RHICommandBuffer* CreateCommandBuffer(const char* name) override;
    void           DestroyCommandBuffer(RHICommandBuffer* cmdBuffer) override;

    void Submit(Swapchain* swapchain, RHICommandBuffer** buffers, size_t bufferCount) override;

    void Present(Swapchain* swapchain) override;

    void WaitForIdle() const override;

    HS_FORCEINLINE void* GetDevice() const { return _device; }
    
    HS_FORCEINLINE virtual ERHIPlatform GetCurrentPlatform() const { return ERHIPlatform::METAL; }

private:
    void* _device = nullptr;
};

HS_NS_END

#endif /* __HS_RHI_CONTEXT_METAL_H__ */
