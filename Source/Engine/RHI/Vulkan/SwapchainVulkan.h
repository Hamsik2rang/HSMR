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
	SwapchainVulkan(const SwapchainInfo& info, RHIContext* rhiContext, VkInstance instance, RHIDeviceVulkan& deviceVulkan, VkSurfaceKHR surface);
	~SwapchainVulkan() override;

	HS_FORCEINLINE uint8          GetMaxFrameCount() const override { return _maxFrameCount; }
	HS_FORCEINLINE uint8          GetCurrentFrameIndex() const override { return _frameIndex; }
	HS_FORCEINLINE CommandBuffer* GetCommandBufferForCurrentFrame() const override { return static_cast<CommandBuffer*>(_commandBufferVKs[_frameIndex]); }
	HS_FORCEINLINE CommandBuffer* GetCommandBufferByIndex(uint8 index) const override { HS_ASSERT(index < _maxFrameCount, "out of index"); return static_cast<CommandBuffer*>(_commandBufferVKs[index]); }
	HS_FORCEINLINE RenderTarget   GetRenderTargetForCurrentFrame() const override { return _renderTargets[_frameIndex]; }

private:
	void setRenderTargets() override;
	void setRenderPass() override;
	void getSwapchainImages();

	void chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkSwapchainKHR _handle = VK_NULL_HANDLE;

	VkSurfaceKHR _surface;
	VkSurfaceFormatKHR _surfaceFormat;
	VkSurfaceCapabilitiesKHR _surfaceCapabilities;
	VkInstance _instance;

	RHIContext* _rhiContext;
	RHIDeviceVulkan& _deviceVulkan;
	uint8 _frameIndex = 0;
	uint8 _maxFrameCount = 3;

	std::vector<VkImage> _vkImages;
	std::vector<VkImageView> _vkImageViews;

	CommandBufferVulkan** _commandBufferVKs;
};

HS_NS_END

#endif