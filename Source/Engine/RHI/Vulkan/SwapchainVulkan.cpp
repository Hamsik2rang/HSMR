#include "Engine/RHI/Vulkan/SwapchainVulkan.h"

#include "Engine/Core/EngineContext.h"

HS_NS_BEGIN

SwapchainVulkan::SwapchainVulkan(const SwapchainInfo& info, RHIDeviceVulkan* pDeviceVulkan, VkSurfaceKHR surface)
	: Swapchain(info)
	, _pDeviceVulkan(pDeviceVulkan)
	, _swapchain(VK_NULL_HANDLE)
	, _surface(surface)
	, _frameIndex(0)
	, _maxFrameCount(3)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*pDeviceVulkan, _surface, &_surfaceCapabilities);
	

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.flags = 0;
	createInfo.surface = _surface;
	createInfo.minImageCount = _surfaceCapabilities.minImageCount;
	createInfo.imageExtent.width = _surfaceCapabilities.currentExtent.width;
	createInfo.imageExtent.height = _surfaceCapabilities.currentExtent.height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	createInfo.pNext = nullptr;
	
}


SwapchainVulkan::~SwapchainVulkan()
{
	if (_swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(*_pDeviceVulkan, _swapchain, nullptr);
		_swapchain = VK_NULL_HANDLE;
	}
}


CommandBuffer* SwapchainVulkan::GetCommandBufferForCurrentFrame() const
{
	return nullptr;
}

CommandBuffer* SwapchainVulkan::GetCommandBufferByIndex(uint8 index) const
{
	return nullptr;
}

RenderTarget SwapchainVulkan::GetRenderTargetForCurrentFrame() const
{
	return RenderTarget();
}

void SwapchainVulkan::setRenderTargets()
{
	// Set render targets for the swapchain
}

void SwapchainVulkan::setRenderPass()
{
	// Set render pass for the swapchain
}



HS_NS_END