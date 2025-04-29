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
	friend class RHIContextVulkan;
private:
	SwapchainVulkan(SwapchainInfo& info, RHIContext* rhiContext, RHIDeviceVulkan& deviceVulkan, VkSurfaceKHR surface);
	~SwapchainVulkan() override;


	HS_FORCEINLINE uint8          GetMaxFrameIndex() const override { return maxFrameCount; }
	HS_FORCEINLINE uint8          GetCurrentFrameIndex() const override { return frameIndex; }
	HS_FORCEINLINE CommandBuffer* GetCommandBufferForCurrentFrame() const override { return static_cast<CommandBuffer*>(commandBufferVKs[frameIndex]); }
	HS_FORCEINLINE CommandBuffer* GetCommandBufferByIndex(uint8 index) const override { HS_ASSERT(index < maxFrameCount, "out of index"); return static_cast<CommandBuffer*>(commandBufferVKs[index]); }
	HS_FORCEINLINE RenderTarget   GetRenderTargetForCurrentFrame() const override { return _renderTargets[frameIndex]; }


	void setRenderTargets() override;
	void setRenderPass() override;

	VkSwapchainKHR handle = VK_NULL_HANDLE;
	
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	RHIContext* rhiContext;
	RHIDeviceVulkan& deviceVulkan;
	uint8 frameIndex = 0;
	uint8 maxFrameCount = 3;

	std::vector<VkImage> vkImages;
	std::vector<VkImageView> vkImageViews;
	
	CommandBufferVulkan** commandBufferVKs;
};

HS_NS_END

#endif