#include "Engine/RHI/Vulkan/SwapchainVulkan.h"

#include "Engine/Platform/Windows/PlatformWindowWindows.h"

#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RHIContextVulkan.h"
#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"


#include <vulkan/vulkan.h>

HS_NS_BEGIN

SwapchainVulkan::SwapchainVulkan(const SwapchainInfo& info, VkSurfaceKHR surface)
	: Swapchain(info)
	, _deviceVulkan(nullptr)
	, surface(surface)
	, handle(VK_NULL_HANDLE)
	, _frameIndex(static_cast<uint8>(-1))
	, _maxFrameCount(3)
	, _isSuspended(true)
	, _isInitialized(false)
{

}

SwapchainVulkan::~SwapchainVulkan()
{
	destroySwapchainVK();
}

void SwapchainVulkan::setRenderTargets()
{
	uint32 swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(_deviceVulkan->logicalDevice, handle, &swapchainImageCount, nullptr);
	HS_ASSERT(static_cast<uint32>(_maxFrameCount) == swapchainImageCount, "Swapchain image count is not same with max frame count.");

	if (vkImages.size() != swapchainImageCount)
	{
		vkImages.resize(swapchainImageCount);
	}
	vkGetSwapchainImagesKHR(_deviceVulkan->logicalDevice, handle, &swapchainImageCount, vkImages.data());

	vkImageViews.resize(vkImages.size());
	_renderTargets.resize(vkImages.size());
	for (int i = 0; i < vkImages.size(); i++)
	{
		VkImageViewCreateInfo colorAttachmentView = {};
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.pNext = NULL;
		colorAttachmentView.format = VK_FORMAT_B8G8R8A8_UNORM;
		colorAttachmentView.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;
		colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorAttachmentView.flags = 0;
		colorAttachmentView.image = vkImages[i];
		VK_CHECK_RESULT(vkCreateImageView(_deviceVulkan->logicalDevice, &colorAttachmentView, nullptr, &vkImageViews[i]));
	}

    RenderTargetInfo info{};
    info.isSwapchainTarget      = true;
	info.swapchain				= this;
    info.colorTextureCount      = 1;
    info.useDepthStencilTexture = false; // TOOD: 선택 가능하면 좋을듯함.

    TextureInfo colorTextureInfo{};
    colorTextureInfo.extent.width         = _info.nativeWindow->width;
    colorTextureInfo.extent.height        = _info.nativeWindow->height;
    colorTextureInfo.extent.depth         = 1;
    colorTextureInfo.format               = EPixelFormat::B8G8A8R8_UNORM;
    colorTextureInfo.usage                = ETextureUsage::COLOR_RENDER_TARGET;
    colorTextureInfo.arrayLength          = 1;
    colorTextureInfo.mipLevel             = 1;
    colorTextureInfo.isCompressed         = false;
    colorTextureInfo.useGenerateMipmap    = false;
    colorTextureInfo.isDepthStencilBuffer = false;
    colorTextureInfo.type                 = ETextureType::TEX_2D;

    info.colorTextureInfos.emplace_back(colorTextureInfo);

    for (auto& renderTarget : _renderTargets)
    {
        renderTarget.Create(info);
    }
}

void SwapchainVulkan::setRenderPass()
{
	Attachment colorAttachment{};
	colorAttachment.format = RHIUtilityVulkan::FromPixelFormat(surfaceFormat.format);
	colorAttachment.clearValue = ClearValue(0.3, 0.3, 0.3, 1.0);
	colorAttachment.loadAction = ELoadAction::CLEAR;
	colorAttachment.storeAction = EStoreAction::STORE;
	colorAttachment.isDepthStencil = false;

	Area renderArea{};
	renderArea.x = 0;
	renderArea.y = 0;
	renderArea.width = _info.nativeWindow->surfaceWidth;
	renderArea.height = _info.nativeWindow->surfaceHeight;

    RenderPassInfo info{};
    info.isSwapchainRenderPass = true;
	info.colorAttachments = { colorAttachment };
    info.colorAttachmentCount  = 1;
	info.renderArea = renderArea;
	info.useDepthStencilAttachment = false;

    _renderPass = hs_engine_get_rhi_context()->CreateRenderPass(info);
}

void SwapchainVulkan::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surfaceFormat = availableFormat;
			return;
		}
	}

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surfaceFormat = availableFormat;
			return;
		}
	}
}

void SwapchainVulkan::getSwapchainImages()
{
	uint32 swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(_deviceVulkan->logicalDevice, handle, &swapchainImageCount, nullptr);
	
	_maxFrameCount = swapchainImageCount;
	vkImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(_deviceVulkan->logicalDevice, handle, &swapchainImageCount, vkImages.data());

	vkImageViews.resize(vkImages.size());
	for (size_t i = 0; i < vkImages.size(); i++)
	{
		if (vkImageViews[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(_deviceVulkan->logicalDevice, vkImageViews[i], nullptr);
			vkImageViews[i] = VK_NULL_HANDLE;
		}

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = vkImages[i];
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = surfaceFormat.format;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(_deviceVulkan->logicalDevice, &viewCreateInfo, nullptr, &vkImageViews[i]));
	}

	return;
}

bool SwapchainVulkan::initSwapchainVK(RHIContextVulkan* rhiContext, VkInstance instance, RHIDeviceVulkan* deviceVulkan)
{
	_deviceVulkan = deviceVulkan;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_deviceVulkan->physicalDevice, surface, &surfaceCapabilities);
	VkSwapchainCreateInfoKHR createInfo{};

	VkExtent2D extent{};
	extent.height = _info.nativeWindow->surfaceHeight;
	extent.width = _info.nativeWindow->surfaceWidth;

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_deviceVulkan->physicalDevice, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> availabieSurfaceFormat;
	if (formatCount != 0)
	{
		availabieSurfaceFormat.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_deviceVulkan->physicalDevice, surface, &formatCount, availabieSurfaceFormat.data());
		chooseSurfaceFormat(availabieSurfaceFormat);
	}

	uint32 presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_deviceVulkan->physicalDevice, surface, &presentModeCount, nullptr);
	HS_ASSERT(presentModeCount > 0, "This surface doesn't have any present mode.");
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(_deviceVulkan->physicalDevice, surface, &presentModeCount, presentModes.data());

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	uint32 desiredNumOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if ((surfaceCapabilities.maxImageCount > 0) && (desiredNumOfSwapchainImages > surfaceCapabilities.maxImageCount))
	{
		desiredNumOfSwapchainImages = surfaceCapabilities.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfaceCapabilities.currentTransform;
	}

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = desiredNumOfSwapchainImages;
	createInfo.imageExtent.width = extent.width;
	createInfo.imageExtent.height = extent.height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform);
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = handle;
	createInfo.pNext = nullptr;

	VK_CHECK_RESULT(vkCreateSwapchainKHR(_deviceVulkan->logicalDevice, &createInfo, nullptr, &handle));

	_maxFrameCount = surfaceCapabilities.minImageCount;
	_commandBufferVKs = new CommandBufferVulkan * [_maxFrameCount];
	syncObjects.imageAvailableSemaphores = new VkSemaphore[_maxFrameCount];
	syncObjects.renderFinishedSemaphores = new VkSemaphore[_maxFrameCount];
	syncObjects.inFlightFences = new VkFence[_maxFrameCount];

	for (uint8 i = 0; i < _maxFrameCount; i++)
	{
		_commandBufferVKs[i] = static_cast<CommandBufferVulkan*>(rhiContext->CreateCommandBuffer());

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;
		VK_CHECK_RESULT(vkCreateSemaphore(_deviceVulkan->logicalDevice, &semaphoreInfo, nullptr, &syncObjects.imageAvailableSemaphores[i]));
		VK_CHECK_RESULT(vkCreateSemaphore(_deviceVulkan->logicalDevice, &semaphoreInfo, nullptr, &syncObjects.renderFinishedSemaphores[i]));

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start in signaled state
		fenceInfo.pNext = nullptr;
		VK_CHECK_RESULT(vkCreateFence(_deviceVulkan->logicalDevice, &fenceInfo, nullptr, &syncObjects.inFlightFences[i]));
	}

	getSwapchainImages();

	setRenderTargets();
	setRenderPass();

	_isInitialized = true;
	return true;
}

void SwapchainVulkan::destroySwapchainVK()
{
	if (!_isInitialized)
	{
		return;
	}

	for (size_t i = 0; i < vkImageViews.size(); i++)
	{
		if (vkImageViews[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(_deviceVulkan->logicalDevice, vkImageViews[i], nullptr);
			vkImageViews[i] = VK_NULL_HANDLE;
		}
	}
	vkImageViews.clear();

	if (handle != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_deviceVulkan->logicalDevice, handle, nullptr);
		handle = VK_NULL_HANDLE;
	}

	_isInitialized = false;
}

HS_NS_END