#include "RHI/Vulkan/VulkanSwapchain.h"

#include "RHI/Vulkan/VulkanUtility.h"
#include "RHI/Vulkan/VulkanContext.h"
#include "RHI/Vulkan/VulkanCommandHandle.h"


#include <vulkan/vulkan.h>

HS_NS_BEGIN

SwapchainVulkan::SwapchainVulkan(const SwapchainInfo& info, VkSurfaceKHR surface)
	: Swapchain(info)
	, _deviceVulkan(nullptr)
	, surface(surface)
	, handle(VK_NULL_HANDLE)
	, _framebuffers(nullptr)
	, _frameIndex(static_cast<uint8>(-1))
	, _maxFrameCount(2)
	, _isSuspended(true)
	, _isInitialized(false)
{

}

SwapchainVulkan::~SwapchainVulkan()
{
	destroySwapchainVK();
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
	info.colorAttachmentCount = 1;
	info.useDepthStencilAttachment = false;

	_renderPass = RHIContext::Get()->CreateRenderPass("Swapchain RenderPass", info);
}

void SwapchainVulkan::setFramebuffers()
{
	HS_ASSERT(_framebuffers == nullptr, "Framebuffer is already exists. you should destroy it before creating new one.");

	_framebuffers = new RHIFramebuffer * [imageVks.size()] { nullptr };

	RHIContext* rhiContext = RHIContext::Get();

	// Framebuffer는 스왑체인 이미지와 1대1 대응이므로 VkImage개수와 동일하게 생성합니다.
	for (uint8 i = 0; i < imageVks.size(); i++)
	{
		TextureInfo tInfo{};
		tInfo.arrayLength = 0;
		tInfo.extent.width = _info.nativeWindow->surfaceWidth;
		tInfo.extent.height = _info.nativeWindow->surfaceHeight;
		tInfo.extent.depth = 1;
		tInfo.format = RHIUtilityVulkan::FromPixelFormat(surfaceFormat.format);
		tInfo.usage = ETextureUsage::COLOR_ATTACHMENT;
		tInfo.isCompressed = false;
		tInfo.useGenerateMipmap = false;
		tInfo.mipLevel = 1;
		tInfo.isSwapchainTexture = true;
		tInfo.type = ETextureType::TEX_2D;
		tInfo.swapchain = this;
		tInfo.byteSize = tInfo.extent.width * tInfo.extent.height * 4; // Assuming 4 bytes per pixel
		tInfo.isDepthStencilBuffer = false;

		RHITexture* texture = rhiContext->CreateTexture("Swapchain Framebffer Texture", nullptr, tInfo);
		
		FramebufferInfo fbInfo{};
		fbInfo.depthStencilBuffer = nullptr;
		fbInfo.resolveBuffer = nullptr;
		fbInfo.isSwapchainFramebuffer = true;
		fbInfo.width = tInfo.extent.width;
		fbInfo.height = tInfo.extent.height;
		fbInfo.renderPass = _renderPass;
		fbInfo.colorBuffers.push_back(texture);

		RHIFramebuffer* framebuffer = rhiContext->CreateFramebuffer("Swapchain Framebuffer", fbInfo);
		_framebuffers[i] = framebuffer;
	}
}

void SwapchainVulkan::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	//for (const auto& availableFormat : availableFormats)
	//{
	//	if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	//	{
	//		surfaceFormat = availableFormat;
	//		return;
	//	}
	//}

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

	imageVks.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(_deviceVulkan->logicalDevice, handle, &swapchainImageCount, imageVks.data());

	imageViewVks.resize(imageVks.size());
	for (size_t i = 0; i < imageVks.size(); i++)
	{
		if (imageViewVks[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(_deviceVulkan->logicalDevice, imageViewVks[i], nullptr);
			imageViewVks[i] = VK_NULL_HANDLE;
		}

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = imageVks[i];
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

		VK_CHECK_RESULT(vkCreateImageView(_deviceVulkan->logicalDevice, &viewCreateInfo, nullptr, &imageViewVks[i]));
	}

	return;
}

bool SwapchainVulkan::initSwapchainVK(VulkanContext* rhiContext, VkInstance instance, VulkanDevice* deviceVulkan)
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

	getSwapchainImages(); 
	_maxFrameCount = desiredNumOfSwapchainImages;

	_commandBufferVKs = new CommandBufferVulkan * [_maxFrameCount];
	syncObjects.imageAvailableSemaphores = new VkSemaphore[_maxFrameCount]{ VK_NULL_HANDLE };
	syncObjects.renderFinishedSemaphores = new VkSemaphore[_maxFrameCount]{ VK_NULL_HANDLE };
	syncObjects.inFlightFences = new VkFence[_maxFrameCount]{ VK_NULL_HANDLE };

	for (uint8 i = 0; i < _maxFrameCount; i++)
	{
		_commandBufferVKs[i] = static_cast<CommandBufferVulkan*>(rhiContext->CreateCommandBuffer("CommandBuffer in Swapchain"));

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
		VK_CHECK_RESULT(vkCreateFence(_deviceVulkan->logicalDevice, &fenceInfo, nullptr, &(syncObjects.inFlightFences[i])));
	}

	setRenderPass();
	setFramebuffers();

	_isInitialized = true;
	return true;
}

void SwapchainVulkan::destroySwapchainVK()
{
	if (!_isInitialized)
	{
		return;
	}
	RHIContext* rhiContext = RHIContext::Get();
	rhiContext->WaitForIdle();

	for (size_t i = 0; i < imageViewVks.size(); i++)
	{
		if (imageViewVks[i] != VK_NULL_HANDLE)
		{
			vkDestroyImageView(_deviceVulkan->logicalDevice, imageViewVks[i], nullptr);
			imageViewVks[i] = VK_NULL_HANDLE;
		}
	}
	imageViewVks.clear();

	if (handle != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(_deviceVulkan->logicalDevice, handle, nullptr);
		handle = VK_NULL_HANDLE;
	}


	if (_renderPass)
	{
		rhiContext->DestroyRenderPass(_renderPass);
		_renderPass = nullptr;
	}
	if (_framebuffers)
	{
		for (size_t i = 0; i < _maxFrameCount; i++)
		{
			if (_framebuffers[i])
			{
				delete _framebuffers[i];
				_framebuffers[i] = nullptr;
			}
		}
		delete[] _framebuffers;
		_framebuffers = nullptr;
	}

	_isInitialized = false;
}

HS_NS_END