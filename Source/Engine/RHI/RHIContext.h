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

	virtual RenderPass* CreateRenderPass(const char* name, const RenderPassInfo& info) = 0;
	virtual void        DestroyRenderPass(RenderPass* renderPass) = 0;

	virtual Framebuffer* CreateFramebuffer(const char* name, const FramebufferInfo& info) = 0;
	virtual void         DestroyFramebuffer(Framebuffer* framebuffer) = 0;

	virtual GraphicsPipeline* CreateGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info) = 0;
	virtual void              DestroyGraphicsPipeline(GraphicsPipeline* pipeline) = 0;

	virtual Shader* CreateShader(const char* name, const ShaderInfo& info, const char* path) = 0;
	virtual Shader* CreateShader(const char* name, const ShaderInfo& info, const char* byteCode, size_t byteCodeSize) = 0;

	virtual void    DestroyShader(Shader* shader) = 0;

	virtual Buffer* CreateBuffer(const char* name, const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) = 0;
	virtual Buffer* CreateBuffer(const char* name, const void* data, size_t dataSize, const BufferInfo& info) = 0;
	virtual void    DestroyBuffer(Buffer* buffer) = 0;

	virtual Texture* CreateTexture(const char* name, void* image, const TextureInfo& info) = 0;
	virtual Texture* CreateTexture(const char* name, void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) = 0;
	virtual void     DestroyTexture(Texture* texture) = 0;

	virtual Sampler* CreateSampler(const char* name, const SamplerInfo& info) = 0;
	virtual void     DestroySampler(Sampler* sampler) = 0;

	virtual ResourceLayout* CreateResourceLayout(const char* name, ResourceBinding* bindings, uint32 bindingCount) = 0;
	virtual void            DestroyResourceLayout(ResourceLayout* resourceLayout) = 0;

	virtual ResourceSet* CreateResourceSet(const char* name, ResourceLayout* resourceLayouts) = 0;
	virtual void         DestroyResourceSet(ResourceSet* dSet) = 0;

	virtual ResourceSetPool*	CreateResourceSetPool(const char* name, uint32 bufferSize, uint32 textureSize) = 0;
	virtual void				DestroyResourceSetPool(ResourceSetPool* pool) = 0;

	virtual CommandPool* CreateCommandPool(const char* name, uint32 queueFamilyIndex = 0) = 0;
	virtual void         DestroyCommandPool(CommandPool* cmdPool) = 0;

	virtual CommandBuffer* CreateCommandBuffer(const char* name) = 0;
	virtual void           DestroyCommandBuffer(CommandBuffer* cmdBuffer) = 0;

	virtual void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) = 0;

	virtual void Present(Swapchain* swapchain) = 0;

	virtual void WaitForIdle() const = 0;
};

HS_NS_END

#endif
