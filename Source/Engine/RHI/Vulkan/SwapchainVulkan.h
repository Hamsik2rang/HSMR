#ifndef __HS_SWAPCHAIN_VULKAN_H__
#define __HS_SWAPCHAIN_VULKAN_H__

#include "Precompile.h"	

#include "Engine/RHI/Swapchain.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"

HS_NS_BEGIN

class CommandBufferVulkan;
class RHIContext;

class SwapchainVulkan final : public Swapchain
{
public:
	SwapchainVulkan(const SwapchainInfo& info, RHIContext* rhiContext, RHIDeviceVulkan& deviceVulkan, VkSurfaceKHR surface);
	~SwapchainVulkan() override;

	uint8 GetMaxFrameCount() const override { return maxFrameCount; }
	uint8 GetCurrentFrameIndex() const override { return frameIndex; }
	CommandBuffer* GetCommandBufferForCurrentFrame() const override;
	CommandBuffer* GetCommandBufferByIndex(uint8 index) const override;
	RenderTarget GetRenderTargetForCurrentFrame() const override;

	void setRenderTargets() override;
	void setRenderPass() override;

	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	RHIContext* rhiContext;
	RHIDeviceVulkan& deviceVulkan;
	VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
	uint8 frameIndex = 0;
	uint8 maxFrameCount = 3;

	CommandBufferVulkan** commandBufferVKs;
};

HS_NS_END

#endif