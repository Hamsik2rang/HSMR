#ifndef __HS_SWAPCHAIN_VULKAN_H__
#define __HS_SWAPCHAIN_VULKAN_H__

#include "Precompile.h"

#include "Core/Log.h"

#include "RHI/Swapchain.h"
#include "RHI/Vulkan/VulkanDevice.h"
#include "RHI/Vulkan/VulkanCommandHandle.h"
#include "RHI/Vulkan/VulkanRenderHandle.h"

HS_NS_BEGIN

class VulkanCommandBuffer;
class RHIContext;

class HS_RHI_API VulkanSwapchain final : public Swapchain
{
public:
    friend class VulkanContext;
    VulkanSwapchain(const SwapchainInfo& info, VkSurfaceKHR surface);
    ~VulkanSwapchain() override;

    HS_FORCEINLINE uint8 GetMaxFrameCount() const override { return _maxFrameCount; }
    HS_FORCEINLINE uint8 GetCurrentFrameIndex() const override { return _frameIndex; }
    HS_FORCEINLINE uint8 GetCurrentImageIndex() const override { return _curImageIndex; }
    HS_FORCEINLINE RHICommandBuffer* GetCommandBufferForCurrentFrame() const override
    {
        return static_cast<RHICommandBuffer*>(_commandBufferVKs[_frameIndex]);
    }

    HS_FORCEINLINE RHICommandBuffer* GetCommandBufferByIndex(uint8 index) const override
    {
        HS_ASSERT(index < _maxFrameCount, "out of index");
        return static_cast<RHICommandBuffer*>(_commandBufferVKs[index]);
    }
    
    HS_FORCEINLINE RHITexture* GetCurrentColorTexture() const override 
    {
        
        if (_curImageIndex > _colorTextures.size())
        {
            HS_LOG(error, "Swapchain wasn't acquired yet.");
            return nullptr;
        }

        return _colorTextures[_curImageIndex]; 
    }
    
    HS_FORCEINLINE EPixelFormat GetColorFormat() const override
    {
        if (_colorTextures.empty())
        {
            return EPixelFormat::Invalid;
        }

        // 스왑체인의 모든 이미지는 같은 포맷으로만 구성되므로 0번 텍스쳐의 포맷을 리턴
        return _colorTextures.front()->info.format;
    }

    VkSwapchainKHR handle = VK_NULL_HANDLE;

    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;

    std::vector<VkImage> imageVks;
    std::vector<VkImageView> imageViewVks;
    struct
    {
        VkSemaphore* imageAvailableSemaphores = nullptr;
        VkSemaphore* renderFinishedSemaphores = nullptr;
        VkFence* inFlightFences = nullptr;
    } syncObjects;

private:
    bool initSwapchainVK(VulkanContext* rhiContext, VkInstance instance, VulkanDevice* deviceVulkan);
    void destroySwapchainVK();

    void setRenderTargets();
    void getSwapchainImages();

    void chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    uint8 _frameIndex     = static_cast<uint8>(-1);
    uint8 _maxFrameCount  = 2;
    uint32 _curImageIndex = static_cast<uint32>(-1);
    VulkanDevice* _deviceVulkan;
    VulkanCommandBuffer** _commandBufferVKs = nullptr;
    std::vector<RHITexture*> _colorTextures;
    bool _isSuspended;
    bool _isInitialized = false;
};

HS_NS_END

#endif
