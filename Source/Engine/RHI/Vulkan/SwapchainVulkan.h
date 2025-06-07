#ifndef __HS_SWAPCHAIN_VULKAN_H__
#define __HS_SWAPCHAIN_VULKAN_H__

#include "Precompile.h"	

#include "Engine/Core/Log.h"

#include "Engine/RHI/Swapchain.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"
#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"

HS_NS_BEGIN

class CommandBufferVulkan;
class RHIContext;

class SwapchainVulkan final : public Swapchain
{
public:
	friend class RHIContextVulkan;
	SwapchainVulkan(const SwapchainInfo& info, VkSurfaceKHR surface);
	~SwapchainVulkan() override;

	HS_FORCEINLINE uint8          GetMaxFrameCount() const override { return _maxFrameCount; }
	HS_FORCEINLINE uint8          GetCurrentFrameIndex() const override { return _frameIndex; }
	HS_FORCEINLINE CommandBuffer* GetCommandBufferForCurrentFrame() const override { return static_cast<CommandBuffer*>(_commandBufferVKs[_frameIndex]); }
	HS_FORCEINLINE CommandBuffer* GetCommandBufferByIndex(uint8 index) const override { HS_ASSERT(index < _maxFrameCount, "out of index"); return static_cast<CommandBuffer*>(_commandBufferVKs[index]); }
	HS_FORCEINLINE RenderTarget   GetRenderTargetForCurrentFrame() const override { return _renderTargets[_frameIndex]; }

	bool initSwapchainVK(RHIContextVulkan* rhiContext, VkInstance instance, RHIDeviceVulkan* deviceVulkan);
	void destroySwapchainVK();

	void setRenderTargets() override;
	void setRenderPass() override;
	void getSwapchainImages();

	void chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkSwapchainKHR handle = VK_NULL_HANDLE;

	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	std::vector<VkImage> vkImages;
	std::vector<VkImageView> vkImageViews;
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
	RHIDeviceVulkan* _deviceVulkan;
	CommandBufferVulkan** _commandBufferVKs;
	bool _isSuspended;
	bool _isInitialized = false;
};

HS_NS_END

#endif