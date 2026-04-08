//
//  VulkanContext.h
//  Engine
//
//  Created by Yongsik Im on 4/11/25.
//
#ifndef __HS_RHI_CONTEXT_VULKAN_H__
#define __HS_RHI_CONTEXT_VULKAN_H__

#include "Precompile.h"

#include "RHI/RHIContext.h"
#include "RHI/Vulkan/VulkanUtility.h"
#include "RHI/Vulkan/VulkanDevice.h"
#include "RHI/Vulkan/VulkanDescriptorPoolAllocator.h"
#include "RHI/Vulkan/VulkanRenderingCache.h"

HS_NS_BEGIN

class HS_RHI_API VulkanContext final : public RHIContext
{
public:
    VulkanContext() = default;
    ~VulkanContext() final;

    bool Initialize() final;
    void Finalize() final;

    void Suspend(Swapchain* swapchain) final;
    void Restore(Swapchain* swapchain) final;

    uint32 AcquireNextImage(Swapchain* swapchain) final;

    Swapchain* CreateSwapchain(SwapchainInfo info) final;
    void DestroySwapchain(Swapchain* swapchain) final;

    RHIGraphicsPipeline* CreateGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info) final;
    void DestroyGraphicsPipeline(RHIGraphicsPipeline* pipeline) final;

    RHIComputePipeline* CreateComputePipeline(const char* name, const ComputePipelineInfo& info) final;
    void DestroyComputePipeline(RHIComputePipeline* pipeline) final;

    RHIShader* CreateShader(const char* name, const ShaderInfo& info, const char* path) final;
    RHIShader* CreateShader(const char* name, const ShaderInfo& info, const char* byteCode, size_t byteCodeSize) final;
    void DestroyShader(RHIShader* shader) final;

    RHIBuffer* CreateBuffer(const char* name, const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) final;
    RHIBuffer* CreateBuffer(const char* name, const void* data, size_t dataSize, const BufferInfo& info) final;
    void DestroyBuffer(RHIBuffer* buffer) final;

    void UpdateBuffer(RHIBuffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize) final;

    RHITexture* CreateTexture(const char* name, void* image, const TextureInfo& info) final;
    RHITexture* CreateTexture(const char* name, void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) final;
    RHITextureMemoryRequirements GetTextureMemoryRequirements(const TextureInfo& info) final;
    RHIHeap* CreateHeap(const RHIHeapInfo& info) final;
    void DestroyHeap(RHIHeap* heap) final;
    RHITexture* CreateTexture(const char* name, const TextureInfo& info, RHIHeap* heap, uint64 offset) final;
    void DestroyTexture(RHITexture* texture) final;

    RHISampler* CreateSampler(const char* name, const SamplerInfo& info) final;
    void DestroySampler(RHISampler* sampler) final;

    RHIResourceLayout* CreateResourceLayout(const char* name, ResourceBinding* bindings, uint32 bindingCount) final;
    void DestroyResourceLayout(RHIResourceLayout* resourceLayout) final;

    RHIResourceSet* CreateResourceSet(const char* name, RHIResourceLayout* resourceLayouts) final;
    void DestroyResourceSet(RHIResourceSet* resourceSet) final;

    RHIResourceSetPool* CreateResourceSetPool(const char* name, uint32 bufferSize, uint32 textureSize) final;
    void DestroyResourceSetPool(RHIResourceSetPool* resourceSetPool) final;

    // RHICommandQueue* CreateCommandQueue() final;
    // void DestroyCommandQueue(RHICommandQueue* cmdQueue) final;

    RHICommandPool* CreateCommandPool(const char* name, uint32 queueFamilyIndex = 0) final;
    void DestroyCommandPool(RHICommandPool* cmdPool) final;

    RHICommandBuffer* CreateCommandBuffer(const char* name) final;
    void DestroyCommandBuffer(RHICommandBuffer* commandBuffer) final;

    void Submit(Swapchain* swapchain, RHICommandBuffer** buffers, size_t bufferCount) final;

    void Present(Swapchain* swapchain) final;

    void WaitForIdle() const final;

    HS_FORCEINLINE ERHIPlatform GetCurrentPlatform() const override { return ERHIPlatform::Vulkan; }
    HS_FORCEINLINE const RHICapabilities& GetCapabilities() const override { return _device.GetCapabilities(); }

    // TODO: ImGui 백엔드 변경되면 없애야합니다.
    HS_FORCEINLINE const VkInstance GetInstance() const { return _instanceVk; }
    HS_FORCEINLINE const VulkanDevice* GetDevice() const { return &(_device); }
    HS_FORCEINLINE VulkanRenderingCache* GetRenderingCache() { return &_renderingCache; }
    HS_FORCEINLINE VkDevice GetVkDevice() const { return _device.logicalDevice; }

    VkRenderPass GetCompatibleRenderPass(const PipelineRenderTargetLayout& layout);
    void CmdBeginRendering(VkCommandBuffer commandBuffer, const RenderingInfo& renderingInfo);
    void CmdEndRendering(VkCommandBuffer commandBuffer, const RenderingInfo& renderingInfo);

private:
    bool createInstance();
    void createDefaultCommandPool();
    VkSurfaceKHR createSurface(const NativeWindow& nativeWindow);
    VkPipeline createGraphicsPipeline(const GraphicsPipelineInfo& info, VkPipelineLayout& outLayout);
    VkPipeline createComputePipeline(const ComputePipelineInfo& info, VkPipelineLayout& outLayout);

    uint32 getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void traisitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height);

    void setDebugObjectName(VkObjectType type, uint64 handle, const char* name);
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, VkDebugUtilsMessengerEXT* pDebugMessenger, const VkAllocationCallbacks* npAllocator);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* npAllocator);

    void cleanup();

    // std::vector<std::string> _supportedInstanceExtensions;
    VkInstance _instanceVk = VK_NULL_HANDLE;
    VulkanDevice _device;
    VkCommandPool _defaultCommandPool = VK_NULL_HANDLE;
    VulkanDescriptorPoolAllocator _descriptorPoolAllocator;
    VulkanRenderingCache _renderingCache;

    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
    bool _isInitialized                      = false;
};

HS_NS_END

#endif
