//
//  RHIContextVulkan.h
//  Engine
//
//  Created by Yongsik Im on 4/11/25.
//
#ifndef __HS_RHI_CONTEXT_VULKAN_H__
#define __HS_RHI_CONTEXT_VULKAN_H__

#include "Precompile.h"

#include "Engine/RHI/RHIContext.h"
#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"
#include "Engine/RHI/Vulkan/DescriptorPoolAllocatorVulkan.h"


HS_NS_BEGIN

class RHIContextVulkan final : public RHIContext
{
public:
	RHIContextVulkan() = default;
	~RHIContextVulkan() override;

	bool Initialize() override;
	void Finalize() override;

	void Suspend(Swapchain* swapchain) override;
	void Restore(Swapchain* swapchain) override;

	uint32 AcquireNextImage(Swapchain* swapchain) override;

	Swapchain* CreateSwapchain(SwapchainInfo info) override;
	void DestroySwapchain(Swapchain* swapchain) override;

	RenderPass* CreateRenderPass(const RenderPassInfo& info) override;
	void DestroyRenderPass(RenderPass* renderPass) override;

	Framebuffer* CreateFramebuffer(const FramebufferInfo& info) override;
	void DestroyFramebuffer(Framebuffer* framebuffer) override;

	GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineInfo& info) override;
	void DestroyGraphicsPipeline(GraphicsPipeline* pipeline) override;

	Shader* CreateShader(const ShaderInfo& info, const char* path) override;
	Shader* CreateShader(const ShaderInfo& info,  const char* byteCode, size_t byteCodeSize) override;
	void DestroyShader(Shader* shader) override;

	Buffer* CreateBuffer(const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) override;
	Buffer* CreateBuffer(const void* data, size_t dataSize, const BufferInfo& info) override;
	void DestroyBuffer(Buffer* buffer) override;

	Texture* CreateTexture(void* image, const TextureInfo& info) override;
	Texture* CreateTexture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) override;
	void DestroyTexture(Texture* texture) override;

	Sampler* CreateSampler(const SamplerInfo& info) override;
	void DestroySampler(Sampler* sampler) override;

	ResourceLayout* CreateResourceLayout(ResourceBinding* bindings, uint32 bindingCount) override;
	void DestroyResourceLayout(ResourceLayout* resourceLayout) override;

	ResourceSet* CreateResourceSet(ResourceLayout* resourceLayouts) override;
	void DestroyResourceSet(ResourceSet* resourceSet) override;

	// CommandQueue* CreateCommandQueue() override;
	// void DestroyCommandQueue(CommandQueue* cmdQueue) override;

	CommandPool* CreateCommandPool(uint32 queueFamilyIndex = 0) override;
	void         DestroyCommandPool(CommandPool* cmdPool) override;

	CommandBuffer* CreateCommandBuffer() override;
	void DestroyCommandBuffer(CommandBuffer* commandBuffer) override;

	Fence* CreateFence() override;
	void DestroyFence(Fence* fence) override;
#ifdef CreateSemaphore
#pragma push_macro("CreateSemaphore")
#undef CreateSemaphore
#endif
	Semaphore* CreateSemaphore() override;
#ifdef CreateSemaphore
#pragma pop_macro("CreateSemaphore")
#endif
	void DestroySemaphore(Semaphore* semaphore) override;

	void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) override;

	void Present(Swapchain* swapchain) override;

	void WaitForIdle() const override;

	// TODO: ImGui 백엔드 변경되면 없애야합니다.
	const VkInstance GetInstance() const { return _instanceVk; }
	const RHIDeviceVulkan* GetDevice() const  { return &(_device); }

private:
	bool createInstance();
	void createDefaultCommandPool();
	VkSurfaceKHR createSurface(const NativeWindow& nativeWindow);
	VkRenderPass createRenderPass(const RenderPassInfo& info);
	VkFramebuffer createFramebuffer(const FramebufferInfo& info);
	VkPipeline createGraphicsPipeline(const GraphicsPipelineInfo& info);
	VkPipeline createComputePipeline(const ComputePipelineInfo& info);

	uint32 getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void traisitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height);

	void setDebugObjectName(VkObjectType type, uint64 handle, const char* name);
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, VkDebugUtilsMessengerEXT* pDebugMessenger, const VkAllocationCallbacks* npAllocator);
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* npAllocator);

	void cleanup();

	//std::vector<std::string> _supportedInstanceExtensions;
	VkInstance _instanceVk = VK_NULL_HANDLE;
	RHIDeviceVulkan _device;
	VkCommandPool _defaultCommandPool = VK_NULL_HANDLE;
	DescriptorPoolAllocatorVulkan _descriptorPoolAllocator;

	VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
	bool _isInitialized = false;
};


HS_NS_END



#endif