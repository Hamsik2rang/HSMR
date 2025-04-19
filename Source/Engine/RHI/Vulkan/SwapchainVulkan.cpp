#include "Engine/RHI/Vulkan/SwapchainVulkan.h"

#include "Engine/RHI/Vulkan/RHIContextVulkan.h"
#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"
#include "Engine/Core/EngineContext.h"

HS_NS_BEGIN

SwapchainVulkan::SwapchainVulkan(const SwapchainInfo& info, RHIContext* rhiContext, RHIDeviceVulkan& deviceVulkan, VkSurfaceKHR surface)
	: Swapchain(info)
	, deviceVulkan(deviceVulkan)
	, vkSwapchain(VK_NULL_HANDLE)
	, surface(surface)
	, frameIndex(0)
	, maxFrameCount(3)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceVulkan.physicalDevice, surface, &surfaceCapabilities);

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = surfaceCapabilities.minImageCount;
	createInfo.imageExtent.width = surfaceCapabilities.currentExtent.width;
	createInfo.imageExtent.height = surfaceCapabilities.currentExtent.height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	createInfo.pNext = nullptr;
	
	vkCreateSwapchainKHR(deviceVulkan, &createInfo, nullptr, &vkSwapchain);

	maxFrameCount = surfaceCapabilities.minImageCount;
	commandBufferVKs = new CommandBufferVulkan*[maxFrameCount];
	for (uint8 i = 0; i < maxFrameCount; i++)
	{
		commandBufferVKs[i] = static_cast<CommandBufferVulkan*>(rhiContext->CreateCommandBuffer());
	}
}

SwapchainVulkan::~SwapchainVulkan()
{
	if (vkSwapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(deviceVulkan, vkSwapchain, nullptr);
		vkSwapchain = VK_NULL_HANDLE;
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