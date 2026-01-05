//
//  RHIContext.h
//  Engine
//
//  Created by Yongsik Im on 2/11/25.
//
#ifndef __HS_RHI_CONTEXT_H__
#define __HS_RHI_CONTEXT_H__

#include "Precompile.h"

#include "RHI/Swapchain.h"

#include "RHI/RHIDefinition.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"
#include "RHI/ResourceHandle.h"

HS_NS_BEGIN

class HS_API RHIContext
{
public:
	RHIContext() = default;
	virtual ~RHIContext() = default;

	virtual bool Initialize() = 0;
	virtual void Finalize() = 0;

	virtual void Suspend(Swapchain* swapchain) = 0;
	virtual void Restore(Swapchain* swapchain) = 0;

	virtual uint32 AcquireNextImage(Swapchain* swapchain) = 0;

	virtual Swapchain* CreateSwapchain(SwapchainInfo info) = 0;
	virtual void DestroySwapchain(Swapchain* swapchain) = 0;

	virtual RHIRenderPass* CreateRenderPass(const char* name, const RenderPassInfo& info) = 0;
	virtual void DestroyRenderPass(RHIRenderPass* renderPass) = 0;

	virtual RHIFramebuffer* CreateFramebuffer(const char* name, const FramebufferInfo& info) = 0;
	virtual void DestroyFramebuffer(RHIFramebuffer* framebuffer) = 0;

	virtual RHIGraphicsPipeline* CreateGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info) = 0;
	virtual void DestroyGraphicsPipeline(RHIGraphicsPipeline* pipeline) = 0;

	virtual RHIComputePipeline* CreateComputePipeline(const char* name, const ComputePipelineInfo& info) = 0;
	virtual void DestroyComputePipeline(RHIComputePipeline* pipeline) = 0;

	virtual RHIShader* CreateShader(const char* name, const ShaderInfo& info, const char* path) = 0;
	virtual RHIShader* CreateShader(const char* name, const ShaderInfo& info, const char* byteCode, size_t byteCodeSize) = 0;
	virtual void DestroyShader(RHIShader* shader) = 0;

	virtual RHIBuffer* CreateBuffer(const char* name, const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) = 0;
	virtual RHIBuffer* CreateBuffer(const char* name, const void* data, size_t dataSize, const BufferInfo& info) = 0;
	virtual void DestroyBuffer(RHIBuffer* buffer) = 0;

	virtual RHITexture* CreateTexture(const char* name, void* image, const TextureInfo& info) = 0;
	virtual RHITexture* CreateTexture(const char* name, void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) = 0;
	virtual void DestroyTexture(RHITexture* texture) = 0;

	virtual RHISampler* CreateSampler(const char* name, const SamplerInfo& info) = 0;
	virtual void DestroySampler(RHISampler* sampler) = 0;

	virtual RHIResourceLayout* CreateResourceLayout(const char* name, ResourceBinding* bindings, uint32 bindingCount) = 0;
	virtual void DestroyResourceLayout(RHIResourceLayout* resourceLayout) = 0;

	virtual RHIResourceSet* CreateResourceSet(const char* name, RHIResourceLayout* resourceLayouts) = 0;
	virtual void DestroyResourceSet(RHIResourceSet* dSet) = 0;

	virtual RHIResourceSetPool* CreateResourceSetPool(const char* name, uint32 bufferSize, uint32 textureSize) = 0;
	virtual void DestroyResourceSetPool(RHIResourceSetPool* pool) = 0;

	virtual RHICommandPool* CreateCommandPool(const char* name, uint32 queueFamilyIndex = 0) = 0;
	virtual void DestroyCommandPool(RHICommandPool* cmdPool) = 0;

	virtual RHICommandBuffer* CreateCommandBuffer(const char* name) = 0;
	virtual void DestroyCommandBuffer(RHICommandBuffer* cmdBuffer) = 0;

	virtual void Submit(Swapchain* swapchain, RHICommandBuffer** buffers, size_t bufferCount) = 0;

	virtual void Present(Swapchain* swapchain) = 0;

	virtual void WaitForIdle() const = 0;
    
    virtual ERHIPlatform GetCurrentPlatform() const = 0;

	static RHIContext* Create(ERHIPlatform platform);
	static RHIContext* Get();
};

extern HS_API RHIContext* g_rhiContext;

HS_NS_END

#endif
