//
//  RHIContext.h
//  Engine
//
//  Created by Yongsik Im on 2/11/25.
//
#ifndef __HS_RHI_CONTEXT_H__
#define __HS_RHI_CONTEXT_H__

#include "Precompile.h"

#include "Engine/RHI/Swapchain.h"

#include "Engine/RHI/RHIDefinition.h"
#include "Engine/RHI/RenderHandle.h"
#include "Engine/RHI/CommandHandle.h"
#include "Engine/RHI/ResourceHandle.h"

HS_NS_BEGIN

class RHIContext
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
	virtual void       DestroySwapchain(Swapchain* swapchain) = 0;

	virtual RenderPass* CreateRenderPass(const RenderPassInfo& info) = 0;
	virtual void        DestroyRenderPass(RenderPass* renderPass) = 0;

	virtual Framebuffer* CreateFramebuffer(const FramebufferInfo& info) = 0;
	virtual void         DestroyFramebuffer(Framebuffer* framebuffer) = 0;

	virtual GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineInfo& info) = 0;
	virtual void              DestroyGraphicsPipeline(GraphicsPipeline* pipeline) = 0;

	virtual Shader* CreateShader(EShaderStage stage, const char* path, const char* entryName, bool isBuiltIn = true) = 0;
	virtual Shader* CreateShader(EShaderStage stage, const char* byteCode, size_t byteCodeSize, const char* entryName, bool isBuitIn = true) = 0;

	virtual void    DestroyShader(Shader* shader) = 0;

	virtual Buffer* CreateBuffer(void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) = 0;
	virtual Buffer* CreateBuffer(void* data, size_t dataSize, const BufferInfo& info) = 0;
	virtual void    DestroyBuffer(Buffer* buffer) = 0;

	virtual Texture* CreateTexture(void* image, const TextureInfo& info) = 0;
	virtual Texture* CreateTexture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) = 0;
	virtual void     DestroyTexture(Texture* texture) = 0;

	virtual Sampler* CreateSampler(const SamplerInfo& info) = 0;
	virtual void     DestroySampler(Sampler* sampler) = 0;

	virtual ResourceLayout* CreateResourceLayout() = 0;
	virtual void            DestroyResourceLayout(ResourceLayout* resourceLayout) = 0;

	virtual ResourceSet* CreateResourceSet() = 0;
	virtual void         DestroyResourceSet(ResourceSet* dSet) = 0;

	virtual ResourceSetPool* CreateResourceSetPool() = 0;
	virtual void             DestroyResourceSetPool(ResourceSetPool* dPool) = 0;

	virtual CommandPool* CreateCommandPool(uint32 queueFamilyIndex = 0) = 0;
	virtual void         DestroyCommandPool(CommandPool* cmdPool) = 0;

	virtual CommandBuffer* CreateCommandBuffer() = 0;
	virtual void           DestroyCommandBuffer(CommandBuffer* cmdBuffer) = 0;

	virtual Fence* CreateFence() = 0;
	virtual void   DestroyFence(Fence* fence) = 0;

#ifdef CreateSemaphore
#pragma push_macro("CreateSemaphore")
#undef CreateSemaphore
#endif
	virtual Semaphore* CreateSemaphore() = 0;
#ifdef CreateSemaphore
#pragma pop_macro("CreateSemaphore")
#endif
	virtual void       DestroySemaphore(Semaphore* semaphore) = 0;

	virtual void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) = 0;

	virtual void Present(Swapchain* swapchain) = 0;

	virtual void WaitForIdle() const = 0;
};

HS_NS_END

#endif
