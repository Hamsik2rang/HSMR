#include "Engine/RHI/Vulkan/SwapchainVulkan.h"

#include "Engine/Platform/Windows/PlatformWindowWindows.h"

#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RHIContextVulkan.h"
#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"


#include <vulkan/vulkan.h>

HS_NS_BEGIN

SwapchainVulkan::SwapchainVulkan(const SwapchainInfo& info, RHIContext* rhiContext, VkInstance instance, RHIDeviceVulkan& deviceVulkan, VkSurfaceKHR surface)
	: Swapchain(info)
	, _instance(instance)
	, _deviceVulkan(deviceVulkan)
	, _handle(VK_NULL_HANDLE)
	, _frameIndex(0)
	, _maxFrameCount(3)
	, _surface(surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceVulkan.physicalDevice, _surface, &_surfaceCapabilities);
	VkSwapchainCreateInfoKHR createInfo{};

	VkExtent2D extent{};
	extent.height = _info.nativeWindow->surfaceHeight;
	extent.width = _info.nativeWindow->surfaceWidth;

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(deviceVulkan.physicalDevice, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> availabieSurfaceFormat;
	if (formatCount != 0)
	{
		availabieSurfaceFormat.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(deviceVulkan.physicalDevice, surface, &formatCount, availabieSurfaceFormat.data());
		chooseSurfaceFormat(availabieSurfaceFormat);
	}


	uint32 presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(deviceVulkan.physicalDevice, _surface, &presentModeCount, nullptr);
	HS_ASSERT(presentModeCount > 0, "This surface doesn't have any present mode.");
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(deviceVulkan.physicalDevice, _surface, &presentModeCount, presentModes.data());

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

	uint32 desiredNumOfSwapchainImages = _surfaceCapabilities.minImageCount + 1;
	if ((_surfaceCapabilities.maxImageCount > 0) && (desiredNumOfSwapchainImages > _surfaceCapabilities.maxImageCount))
	{
		desiredNumOfSwapchainImages = _surfaceCapabilities.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (_surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = _surfaceCapabilities.currentTransform;
	}

	//deviceVulkan.queueFamilyIndices

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.flags = 0;
	createInfo.surface = _surface;
	createInfo.minImageCount = desiredNumOfSwapchainImages;
	createInfo.imageExtent.width = extent.width;
	createInfo.imageExtent.height = extent.height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageColorSpace = _surfaceFormat.colorSpace;
	createInfo.imageFormat = _surfaceFormat.format;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform);
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = _handle;
	createInfo.pNext = nullptr;
	
	VK_CHECK_RESULT(vkCreateSwapchainKHR(deviceVulkan, &createInfo, nullptr, &_handle));

	_maxFrameCount = _surfaceCapabilities.minImageCount;
	_commandBufferVKs = new CommandBufferVulkan*[_maxFrameCount];
	for (uint8 i = 0; i < _maxFrameCount; i++)
	{
		_commandBufferVKs[i] = static_cast<CommandBufferVulkan*>(rhiContext->CreateCommandBuffer());
	}

	getSwapchainImages();
}

SwapchainVulkan::~SwapchainVulkan()
{
	for (size_t i = 0; i < _vkImageViews.size(); i++)
	{
		if (_vkImageViews[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(_deviceVulkan, _vkImageViews[i], nullptr);
			_vkImageViews[i] = VK_NULL_HANDLE;
		}
	}
	_vkImageViews.clear();

	if (_handle != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_deviceVulkan, _handle, nullptr);
		_handle = VK_NULL_HANDLE;
	}
}

void SwapchainVulkan::setRenderTargets()
{
	uint32 swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(_deviceVulkan, _handle, &swapchainImageCount, nullptr);
	HS_ASSERT(static_cast<uint32>(_maxFrameCount) == swapchainImageCount, "Swapchain image count is not same with max frame count.");

	_vkImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(_deviceVulkan, _handle, &swapchainImageCount, _vkImages.data());

	_vkImageViews.resize(_vkImages.size());
	_renderTargets.resize(_vkImages.size());
	for (int i = 0; i < _vkImages.size(); i++)
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
		colorAttachmentView.image = _vkImages[i];
		VK_CHECK_RESULT(vkCreateImageView(_deviceVulkan, &colorAttachmentView, nullptr, &_vkImageViews[i]));
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
    colorTextureInfo.usage                = ETextureUsage::RENDER_TARGET;
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
    //RenderPassInfo info{};
    //info.isSwapchainRenderPass = true;
    //info.colorAttachmentCount  = 1;
    //Attachment colorAttachment{};
    //colorAttachment.format         = EPixelFormat::B8G8A8R8_UNORM;
    //colorAttachment.clearValue     = ClearValue(0.5, 0.5, 0.5, 1.0);
    //colorAttachment.loadAction     = ELoadAction::CLEAR;
    //colorAttachment.storeAction    = EStoreAction::STORE;
    //colorAttachment.isDepthStencil = false;

    //_renderPass = hs_engine_get_rhi_context()->CreateRenderPass(info);
}

void SwapchainVulkan::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			_surfaceFormat = availableFormat;
			return;
		}
	}

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			_surfaceFormat = availableFormat;
			return;
		}
	}
}

void SwapchainVulkan::getSwapchainImages()
{
	uint32 swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(_deviceVulkan, _handle, &swapchainImageCount, nullptr);
	
	_maxFrameCount = swapchainImageCount;
	_vkImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(_deviceVulkan, _handle, &swapchainImageCount, _vkImages.data());

	_vkImageViews.resize(_vkImages.size());
	for (size_t i = 0; i < _vkImages.size(); i++)
	{
		if (_vkImageViews[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(_deviceVulkan, _vkImageViews[i], nullptr);
			_vkImageViews[i] = VK_NULL_HANDLE;
		}

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = _vkImages[i];
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = _surfaceFormat.format;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(_deviceVulkan, &viewCreateInfo, nullptr, &_vkImageViews[i]));
	}

	return;
}


HS_NS_END