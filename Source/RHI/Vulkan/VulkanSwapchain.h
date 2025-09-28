#ifndef __HS_SWAPCHAIN_VULKAN_H__
#define __HS_SWAPCHAIN_VULKAN_H__

#include "Precompile.h"	

#include "Core/Log.h"

#include "RHI/Swapchain.h"
#include "RHI/Vulkan/VulkanDevice.h"
#include "RHI/Vulkan/VulkanCommandHandle.h"
#include "RHI/Vulkan/VulkanRenderHandle.h"

HS_NS_BEGIN

class CommandBufferVulkan;
class RHIContext;

class HS_RHI_API SwapchainVulkan final : public Swapchain
{
public:
	friend class VulkanContext;
	SwapchainVulkan(const SwapchainInfo& info, VkSurfaceKHR surface);
	~SwapchainVulkan() override;

	HS_FORCEINLINE uint8          GetMaxFrameCount() const override { return _maxFrameCount; }
	HS_FORCEINLINE uint8          GetCurrentFrameIndex() const override { return _frameIndex; }
	HS_FORCEINLINE RHICommandBuffer* GetCommandBufferForCurrentFrame() const override { return static_cast<RHICommandBuffer*>(_commandBufferVKs[_frameIndex]); }
	HS_FORCEINLINE RHICommandBuffer* GetCommandBufferByIndex(uint8 index) const override { HS_ASSERT(index < _maxFrameCount, "out of index"); return static_cast<RHICommandBuffer*>(_commandBufferVKs[index]); }
	HS_FORCEINLINE RHIFramebuffer*   GetFramebufferForCurrentFrame() const override { return _framebuffers[_curImageIndex]; }
	HS_FORCEINLINE uint32		  GetCurrentImageIndex() const { return _curImageIndex; }

	bool initSwapchainVK(VulkanContext* rhiContext, VkInstance instance, VulkanDevice* deviceVulkan);
	void destroySwapchainVK();

	void setFramebuffers();
	void getSwapchainImages();

	void chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkSwapchainKHR handle = VK_NULL_HANDLE;

	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	std::vector<VkImage> imageVks;
	std::vector<VkImageView> imageViewVks;
	struct
	{
		VkSemaphore* imageAvailableSemaphores;
		VkSemaphore* renderFinishedSemaphores;
		VkFence* inFlightFences;
	}syncObjects;

private:
	uint8 _frameIndex = static_cast<uint8>(-1);
	uint8 _maxFrameCount = 3;
	uint32 _curImageIndex = static_cast<uint32>(-1);
	VulkanDevice* _deviceVulkan;
	CommandBufferVulkan** _commandBufferVKs;
	RHIFramebuffer** _framebuffers;
	bool _isSuspended;
	bool _isInitialized = false;
};

HS_NS_END

#endif