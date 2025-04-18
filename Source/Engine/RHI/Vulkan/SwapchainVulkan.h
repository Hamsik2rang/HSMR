#ifndef __HS_SWAPCHAIN_VULKAN_H__
#define __HS_SWAPCHAIN_VULKAN_H__

#include "Precompile.h"	

#include "Engine/RHI/Swapchain.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"

HS_NS_BEGIN

class RHIDeviceVulkan;
class CommandBufferVulkan;

class SwapchainVulkan final : public Swapchain
{
public:
	SwapchainVulkan(const SwapchainInfo& info, RHIDeviceVulkan* pDeviceVulkan, VkSurfaceKHR surface);
	~SwapchainVulkan() override;

	uint8 GetMaxFrameCount() const override { return _maxFrameCount; }
	uint8 GetCurrentFrameIndex() const override { return _frameIndex; }
	CommandBuffer* GetCommandBufferForCurrentFrame() const override;
	CommandBuffer* GetCommandBufferByIndex(uint8 index) const override;
	RenderTarget GetRenderTargetForCurrentFrame() const override;

private:
	void setRenderTargets() override;
	void setRenderPass() override;

	VkSurfaceKHR _surface;
	VkSurfaceCapabilitiesKHR _surfaceCapabilities;

	RHIDeviceVulkan* _pDeviceVulkan;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	uint8 _frameIndex = 0;
	uint8 _maxFrameCount = 3;

	CommandBufferVulkan* _commandBuffers;
};

HS_NS_END

#endif