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
	~RHIContextVulkan() final;

	bool Initialize() final;
	void Finalize() final;

	void Suspend(Swapchain* swapchain) final;
	void Restore(Swapchain* swapchain) final;

	uint32 AcquireNextImage(Swapchain* swapchain) final;

	Swapchain* CreateSwapchain(SwapchainInfo info) final;
	void DestroySwapchain(Swapchain* swapchain) final;

	RenderPass* CreateRenderPass(const char* name, const RenderPassInfo& info) final;
	void DestroyRenderPass(RenderPass* renderPass) final;

	Framebuffer* CreateFramebuffer(const char* name, const FramebufferInfo& info) final;
	void DestroyFramebuffer(Framebuffer* framebuffer) final;

	GraphicsPipeline* CreateGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info) final;
	void DestroyGraphicsPipeline(GraphicsPipeline* pipeline) final;

	Shader* CreateShader(const char* name, const ShaderInfo& info, const char* path) final;
	Shader* CreateShader(const char* name, const ShaderInfo& info,  const char* byteCode, size_t byteCodeSize) final;
	void DestroyShader(Shader* shader) final;

	Buffer* CreateBuffer(const char* name, const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) final;
	Buffer* CreateBuffer(const char* name, const void* data, size_t dataSize, const BufferInfo& info) final;
	void DestroyBuffer(Buffer* buffer) final;

	Texture* CreateTexture(const char* name, void* image, const TextureInfo& info) final;
	Texture* CreateTexture(const char* name, void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) final;
	void DestroyTexture(Texture* texture) final;

	Sampler* CreateSampler(const char* name, const SamplerInfo& info) final;
	void DestroySampler(Sampler* sampler) final;

	ResourceLayout* CreateResourceLayout(const char* name, ResourceBinding* bindings, uint32 bindingCount) final;
	void DestroyResourceLayout(ResourceLayout* resourceLayout) final;

	ResourceSet* CreateResourceSet(const char* name, ResourceLayout* resourceLayouts) final;
	void DestroyResourceSet(ResourceSet* resourceSet) final;

	ResourceSetPool* CreateResourceSetPool(const char* name, uint32 bufferSize, uint32 textureSize) final;
	void DestroyResourceSetPool(ResourceSetPool* resourceSetPool) final;

	// CommandQueue* CreateCommandQueue() final;
	// void DestroyCommandQueue(CommandQueue* cmdQueue) final;

	CommandPool* CreateCommandPool(const char* name, uint32 queueFamilyIndex = 0) final;
	void         DestroyCommandPool(CommandPool* cmdPool) final;

	CommandBuffer* CreateCommandBuffer(const char* name) final;
	void DestroyCommandBuffer(CommandBuffer* commandBuffer) final;

	void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) final;

	void Present(Swapchain* swapchain) final;

	void WaitForIdle() const final;

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