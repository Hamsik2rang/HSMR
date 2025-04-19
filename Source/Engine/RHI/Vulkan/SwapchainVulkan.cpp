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

	setRenderTargets();
	setRenderPass();
}

SwapchainVulkan::~SwapchainVulkan()
{
	if (vkSwapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(deviceVulkan, vkSwapchain, nullptr);
		vkSwapchain = VK_NULL_HANDLE;
	}
}

void SwapchainVulkan::setRenderTargets()
{
    RenderTargetInfo info{};
    info.isSwapchainTarget      = true;
    info.colorTextureCount      = 1;
    info.useDepthStencilTexture = false; // TOOD: 선택 가능하면 좋을듯함.

    TextureInfo colorTextureInfo{};
    colorTextureInfo.extent.width         = _info.width;
    colorTextureInfo.extent.height        = _info.height;
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
    RenderPassInfo info{};
    info.isSwapchainRenderPass = true;
    info.colorAttachmentCount  = 1;
    Attachment colorAttachment{};
    colorAttachment.format         = EPixelFormat::B8G8A8R8_UNORM;
    colorAttachment.clearValue     = ClearValue(0.5, 0.5, 0.5, 1.0);
    colorAttachment.loadAction     = ELoadAction::CLEAR;
    colorAttachment.storeAction    = EStoreAction::STORE;
    colorAttachment.isDepthStencil = false;

    _renderPass = hs_engine_get_rhi_context()->CreateRenderPass(info);
}

HS_NS_END