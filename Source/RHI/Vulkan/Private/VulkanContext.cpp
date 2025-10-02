#include "RHI/Vulkan/VulkanContext.h"

#include "RHI/Vulkan/VulkanCommandHandle.h"
#include "RHI/Vulkan/VulkanRenderHandle.h"
#include "RHI/Vulkan/VulkanResourceHandle.h"
#include "RHI/Vulkan/VulkanUtility.h"
#include "RHI/Vulkan/VulkanDevice.h"
#include "RHI/Vulkan/VulkanSwapchain.h"

#include "Core/Utility/StringUtility.h"

#include "HAL/NativeWindow.h"
#include "HAL/FileSystem.h"


static const std::vector<const char*> s_validationLayers =
{
#ifdef _DEBUG
	"VK_LAYER_KHRONOS_validation",
#endif
};

#ifdef _DEBUG
static constexpr bool s_enableValidationLayers = true;
#else
static constexpr bool enableValidationLayers = false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL hs_rhi_vk_report_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	const char* name = pCallbackData->pObjects->pObjectName;
	name = (name != nullptr) ? name : "Unknown";
	std::string oldMessage(pCallbackData->pMessage);
	std::string newMessage;
	
	size_t ptr = 0;
	while (ptr < oldMessage.size())
	{
		size_t nextPtr = std::string::npos;
		nextPtr = oldMessage.find("[]", ptr);
		if (nextPtr == std::string::npos)
		{
			if (ptr < oldMessage.size())
			{
				newMessage.append(oldMessage.substr(ptr));
			}
			break;
		}
		newMessage.append(oldMessage.substr(ptr, nextPtr - ptr));
		newMessage.append(HS::StringUtil::Format("[%s]", name));
		ptr = nextPtr + 2 /*strlen("[]")*/;
	}

	if (messageType & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		HS_LOG(crash, "ERROR: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, newMessage.c_str());
	}
	else if (messageType & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		HS_LOG(crash, "WARNING: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, newMessage.c_str());
	}
	else if (messageType & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		HS_LOG(warning, "PERFORMANCE WARNING: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, newMessage.c_str());
	}
	else if (messageType & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		HS_LOG(debug, "INFO: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, newMessage.c_str());
	}
	else if (messageType & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		HS_LOG(debug, "DEBUG: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, newMessage.c_str());
	}

	return VK_FALSE;
}

HS_NS_BEGIN

VulkanContext::~VulkanContext()
{
	Finalize();
}

bool VulkanContext::Initialize()
{
	if (false == createInstance())
	{
		return false;
	}

	if (false == _device.Create(_instanceVk))
	{
		return false;
	}

	createDefaultCommandPool();

	std::vector<DescriptorPoolAllocatorVulkan::PoolSizeRatio> ratios =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
	};

	_descriptorPoolAllocator.Initialize(_instanceVk, &_device, 1000, ratios);

	_isInitialized = true;

	return true;
}

void VulkanContext::Finalize()
{
	if (!_isInitialized)
	{
		return;
	}

	// Cleanup Vulkan resources
	if (_defaultCommandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(_device, _defaultCommandPool, nullptr);
		_defaultCommandPool = VK_NULL_HANDLE;
	}
	if (_instanceVk != VK_NULL_HANDLE)
	{
		vkDestroyInstance(_instanceVk, nullptr);
		_instanceVk = VK_NULL_HANDLE;
	}

	_isInitialized = false;
}

void VulkanContext::Suspend(Swapchain* swapchain)
{
	HS_ASSERT(swapchain != nullptr, "Swapchain is null in VulkanContext::Suspend");

	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	if (swapchainVK->_isSuspended)
	{
		return;
	}

	swapchainVK->_isSuspended = true;
}

void VulkanContext::Restore(Swapchain* swapchain)
{
	HS_ASSERT(swapchain != nullptr, "Swapchain is null in VulkanContext::Restore");

	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	if (!swapchainVK->_isSuspended)
	{
		return;
	}

	const RHIFramebuffer* swFramebuffer = swapchainVK->_framebuffers[0];

	// Resize 혹은 Maximize된 상황
	if (swFramebuffer->info.width != swapchainVK->_info.nativeWindow->surfaceWidth ||
		swFramebuffer->info.height != swapchainVK->_info.nativeWindow->surfaceHeight)
	{
		swapchainVK->destroySwapchainVK();
		swapchainVK->initSwapchainVK(this, _instanceVk, &_device);
	}

	swapchainVK->_isSuspended = false;
}

uint32 VulkanContext::AcquireNextImage(Swapchain* swapchain)
{
	HS_ASSERT(swapchain != nullptr, "Swapchain is null in VulkanContext::AcquireNextImage");


	// Acquire the next image from the swapchain
	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	swapchainVK->_frameIndex = static_cast<uint8>(swapchainVK->_frameIndex + 1) % swapchainVK->_maxFrameCount;
	
	const uint8 curframeIndex = swapchainVK->_frameIndex % swapchainVK->_maxFrameCount;

	vkWaitForFences(_device, 1, &swapchainVK->syncObjects.inFlightFences[curframeIndex], VK_TRUE, UINT64_MAX);
	vkResetFences(_device, 1, &swapchainVK->syncObjects.inFlightFences[curframeIndex]);

	uint32 imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(_device, swapchainVK->handle,
		UINT64_MAX, // Timeout
		swapchainVK->syncObjects.imageAvailableSemaphores[curframeIndex],
		VK_NULL_HANDLE, // Fence
		&(swapchainVK->_curImageIndex));

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		// Swapchain is out of date, need to recreate it
		HS_LOG(warning, "Swapchain is out of date at frame %d, acquired image index would be %d", curframeIndex, swapchainVK->_curImageIndex);
		return UINT32_MAX; // Indicate that the swapchain needs to be recreated
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		HS_LOG(crash, "Failed to acquire next image from swapchain: %d", result);
		return UINT32_MAX; // Indicate an error
	}

	CommandBufferVulkan* cmdBufferVK = static_cast<CommandBufferVulkan*>(swapchainVK->GetCommandBufferForCurrentFrame());
	vkResetCommandBuffer(cmdBufferVK->handle, 0);

	return swapchainVK->_curImageIndex;
}

Swapchain* VulkanContext::CreateSwapchain(SwapchainInfo info)
{
	VkSurfaceKHR surface = createSurface(*info.nativeWindow);
	SwapchainVulkan* swapchainVK = new SwapchainVulkan(info, surface);
	swapchainVK->initSwapchainVK(this, _instanceVk, &_device);

	return static_cast<Swapchain*>(swapchainVK);
}

void VulkanContext::DestroySwapchain(Swapchain* swapchain)
{
	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	swapchainVK->destroySwapchainVK(); //소멸자에서 호출되지만 컨벤션 통일을 위해 명시적 호출함.

	delete swapchainVK;
}

RHIRenderPass* VulkanContext::CreateRenderPass(const char* name, const RenderPassInfo& info)
{
	RenderPassVulkan* renderPassVK = new RenderPassVulkan(name, info);

	renderPassVK->handle = createRenderPass(info);
	if (renderPassVK->handle == VK_NULL_HANDLE)
	{
		HS_LOG(crash, "Failed to create Vulkan render pass.");
	}

	setDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, reinterpret_cast<uint64>(renderPassVK->handle), name);

	return renderPassVK;
}

void VulkanContext::DestroyRenderPass(RHIRenderPass* renderPass)
{
	RenderPassVulkan* renderPassVK = static_cast<RenderPassVulkan*>(renderPass);
	if (renderPassVK->handle != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(_device, renderPassVK->handle, nullptr);
		renderPassVK->handle = VK_NULL_HANDLE;
	}
	delete renderPassVK; // Delete the RenderPassVulkan object
}

RHIFramebuffer* VulkanContext::CreateFramebuffer(const char* name, const FramebufferInfo& info)
{
	HS_ASSERT(info.renderPass->info.colorAttachmentCount == info.colorBuffers.size(), "Framebuffer Info is not matched with RenderPass Info");

	FramebufferVulkan* framebufferVK = new FramebufferVulkan(name, info);

	size_t attachmentSize = static_cast<uint32>(info.colorBuffers.size());
	std::vector<VkImageView> attachments(attachmentSize);
	for (size_t i = 0; i < attachmentSize; i++)
	{
		TextureVulkan* textureVK = static_cast<TextureVulkan*>(info.colorBuffers[i]);
		if (textureVK && textureVK->imageViewVk)
		{
			attachments[i] = textureVK->imageViewVk;
		}
		else
		{
			HS_LOG(crash, "Invalid color buffer at index %d", i);
			attachments[i] = VK_NULL_HANDLE;
		}
	}
	if (info.renderPass->info.useDepthStencilAttachment && info.depthStencilBuffer != nullptr)
	{
		attachmentSize++;
		TextureVulkan* textureVK = static_cast<TextureVulkan*>(info.depthStencilBuffer);
		attachments.push_back(textureVK->imageViewVk);
	}

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.renderPass = static_cast<RenderPassVulkan*>(info.renderPass)->handle;
	createInfo.attachmentCount = static_cast<uint32>(attachmentSize);
	createInfo.pAttachments = attachments.data();
	createInfo.width = info.width;
	createInfo.height = info.height;
	createInfo.layers = 1;

	VkFramebuffer framebufferVk;
	vkCreateFramebuffer(_device, &createInfo, nullptr, &framebufferVk);
	framebufferVK->handle = framebufferVk;

	setDebugObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, reinterpret_cast<uint64>(framebufferVk), name);

	return static_cast<RHIFramebuffer*>(framebufferVK);
}

void VulkanContext::DestroyFramebuffer(RHIFramebuffer* framebuffer)
{
	// Destroy the Vulkan framebuffer
	FramebufferVulkan* framebufferVK = static_cast<FramebufferVulkan*>(framebuffer);
	if (framebufferVK->handle != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(_device, framebufferVK->handle, nullptr);
		framebufferVK->handle = VK_NULL_HANDLE;
	}
	delete framebufferVK;
}

RHIGraphicsPipeline* VulkanContext::CreateGraphicsPipeline(const char* name, const GraphicsPipelineInfo& info)
{
	GraphicsPipelineVulkan* pipelineVK = new GraphicsPipelineVulkan(name, info);

	VkPipeline pipelineVk = createGraphicsPipeline(info);
	pipelineVK->handle = pipelineVk;

	setDebugObjectName(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64>(pipelineVk), name);

	return static_cast<RHIGraphicsPipeline*>(pipelineVK);
}

void VulkanContext::DestroyGraphicsPipeline(RHIGraphicsPipeline* pipeline)
{
	// Destroy the Vulkan graphics pipeline
	GraphicsPipelineVulkan* pipelineVK = static_cast<GraphicsPipelineVulkan*>(pipeline);
	if (pipelineVK->handle != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(_device, pipelineVK->handle, nullptr);
		pipelineVK->handle = VK_NULL_HANDLE;
	}

	delete pipelineVK;
}

RHIShader* VulkanContext::CreateShader(const char* name, const ShaderInfo& info, const char* path)
{
	FileHandle fileHandle = nullptr;
	if (!FileSystem::Open(path, EFileAccess::READ_ONLY, fileHandle))
	{
		HS_LOG(error, "Fail to open Shader: %s", path);
		return nullptr;
	}

	auto size = FileSystem::GetSize(fileHandle);
	if (size == 0)
	{
		HS_LOG(error, "Shader file is empty: %s", path);
		FileSystem::Close(fileHandle);
		return nullptr;
	}

	char* byteCode = new char[size];
	size_t byteCodeSize = FileSystem::Read(fileHandle, byteCode, size);

	FileSystem::Close(fileHandle);

	// Create a Vulkan shader
	ShaderVulkan* shaderVK = static_cast<ShaderVulkan*>(CreateShader(name, info, byteCode, byteCodeSize));

	delete[] byteCode;

	setDebugObjectName(VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64>(shaderVK->handle), name);

	return shaderVK;
}

RHIShader* VulkanContext::CreateShader(const char* name, const ShaderInfo& info, const char* byteCode, size_t byteCodeSize)
{
	// Create a Vulkan shader from byte code
	VkShaderModuleCreateInfo shaderCreateInfo{};
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = byteCodeSize;
	shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode);
	shaderCreateInfo.pNext = nullptr;

	VkShaderModule shaderModule;
	VK_CHECK_RESULT(vkCreateShaderModule(_device, &shaderCreateInfo, nullptr, &shaderModule));
	if (shaderModule == VK_NULL_HANDLE)
	{
		HS_LOG(error, "Failed to create shader module.");
		return nullptr;
	}

	ShaderVulkan* shaderVK = new ShaderVulkan(name, info);
	shaderVK->handle = shaderModule;

	shaderVK->stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderVK->stageInfo.stage = RHIUtilityVulkan::RHIUtilityVulkan::ToShaderStageFlags(info.stage);
	shaderVK->stageInfo.pName = info.entryName;
	shaderVK->stageInfo.module = shaderVK->handle;

	setDebugObjectName(VK_OBJECT_TYPE_SHADER_MODULE, reinterpret_cast<uint64>(shaderVK->handle), name);

	return static_cast<RHIShader*>(shaderVK);
}

void VulkanContext::DestroyShader(RHIShader* shader)
{
	ShaderVulkan* shaderVulkan = static_cast<ShaderVulkan*>(shader);
	if (shaderVulkan->handle)
	{
		vkDestroyShaderModule(_device, shaderVulkan->handle, nullptr);
		shaderVulkan->handle = VK_NULL_HANDLE;
	}
}

RHIBuffer* VulkanContext::CreateBuffer(const char* name, const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption)
{
	BufferInfo info{};
	info.usage = usage;
	info.memoryOption = memoryOption;

	return CreateBuffer(name, data, dataSize, info);
}

RHIBuffer* VulkanContext::CreateBuffer(const char* name, const void* data, size_t dataSize, const BufferInfo& info)
{
	VkBuffer bufferVk;
	// Create a Vulkan buffer
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = dataSize;
	createInfo.usage = RHIUtilityVulkan::ToBufferUsage(info.usage);
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.pQueueFamilyIndices = &_device.queueFamilyIndices.graphics;

	VK_CHECK_RESULT(vkCreateBuffer(_device, &createInfo, nullptr, &bufferVk));

	VkMemoryRequirements memoryRequirement{};
	vkGetBufferMemoryRequirements(_device, bufferVk, &memoryRequirement);

	VkMemoryPropertyFlags properties{};
	switch (info.memoryOption)
	{
	case EBufferMemoryOption::STATIC:
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	case EBufferMemoryOption::MAPPED:
		properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		break;
	case EBufferMemoryOption::DYNAMIC:
		properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	}

	VkDeviceMemory bufferMemory;
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirement.size;
	allocInfo.memoryTypeIndex = getMemoryTypeIndex(memoryRequirement.memoryTypeBits, properties);

	vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory);

	if (data != nullptr)
	{
		void* mapped;
		VK_CHECK_RESULT(vkMapMemory(_device, bufferMemory, 0, dataSize, 0, &mapped));
		memcpy(mapped, data, dataSize);
		vkUnmapMemory(_device, bufferMemory);
	}

	VK_CHECK_RESULT(vkBindBufferMemory(_device, bufferVk, bufferMemory, 0));

	BufferVulkan* bufferVK = new BufferVulkan(name, info);
	bufferVK->handle = bufferVk;
	bufferVK->memory = bufferMemory;

	setDebugObjectName(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64>(bufferVk), name);

	return static_cast<RHIBuffer*>(bufferVK);
}

void VulkanContext::DestroyBuffer(RHIBuffer* buffer)
{
	BufferVulkan* bufferVK = static_cast<BufferVulkan*>(buffer);
	if (bufferVK->handle != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(_device, bufferVK->handle, nullptr);
		bufferVK->handle = VK_NULL_HANDLE;
	}
	if (bufferVK->memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(_device, bufferVK->memory, nullptr);
		bufferVK->memory = VK_NULL_HANDLE;
	}

	delete bufferVK;
}

RHITexture* VulkanContext::CreateTexture(const char* name, void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage)
{
	// Create a Vulkan texture with specific parameters
	TextureInfo info{};
	info.format = format;
	info.type = type;
	info.usage = usage;
	info.extent.width = width;
	info.extent.height = height;

	return CreateTexture(name, image, info);
}

RHITexture* VulkanContext::CreateTexture(const char* name, void* image, const TextureInfo& info)
{
	bool isColorRenderTarget = ((info.usage & ETextureUsage::COLOR_ATTACHMENT) != 0);
	bool isSwapchainTexture = info.isSwapchainTexture;
	HS_ASSERT((isColorRenderTarget ^ info.isDepthStencilBuffer), "Texture cannot be both color render target and depth stencil buffer.");
	HS_ASSERT(!(info.isSwapchainTexture ^ (info.swapchain != nullptr)), "Texture swapchain mismatch. Texture isSwapchainTexture must match with swapchain.");

	// Swapchain이면 vkGetSwapchainImagesKHR를 통해서 가져온 VkImage와 해당 핸들로 만든 VkImageView를 사용해야 합니다.
	if (isSwapchainTexture)
	{
		SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(info.swapchain);
		for (uint8 i = 0; i < swapchainVK->imageVks.size(); i++)
		{
			if (swapchainVK->_framebuffers[i] == nullptr)
			{
				TextureVulkan* textureVK = new TextureVulkan(name, info);
				textureVK->handle = swapchainVK->imageVks[i];
				textureVK->imageViewVk = swapchainVK->imageViewVks[i];
				textureVK->memoryVk = VK_NULL_HANDLE; // Swapchain textures do not have dedicated memory
				textureVK->layoutVk = VK_IMAGE_LAYOUT_UNDEFINED; // Swapchain images start in undefined layout

				return static_cast<RHITexture*>(textureVK);
			}
		}
		HS_ASSERT(false, "No available swapchain Image for texture creation. Are Framebuffers full?");
	}

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t i = 0; i < 1/* TODO: Mip Levels*/; i++)
	{
		// Setup a buffer image copy structure for the current mip level
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = info.extent.width >> i;
		bufferCopyRegion.imageExtent.height = info.extent.height >> i;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;
		bufferCopyRegions.push_back(bufferCopyRegion);
	}

	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = RHIUtilityVulkan::ToImageType(info.type);
	imageCreateInfo.format = RHIUtilityVulkan::ToPixelFormat(info.format);
	imageCreateInfo.usage = RHIUtilityVulkan::ToTextureUsage(info.usage);
	imageCreateInfo.extent.width = info.extent.width;
	imageCreateInfo.extent.height = info.extent.height;
	imageCreateInfo.extent.depth = 1; // Assuming 2D texture
	imageCreateInfo.arrayLayers = info.type == ETextureType::TEX_CUBE ? 6 : 1; // Assuming single layer
	imageCreateInfo.mipLevels = 1; // TODO: Support mipmaps
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Support MSAA
	imageCreateInfo.tiling = (imageCreateInfo.imageType == VK_IMAGE_TYPE_1D) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = initialLayout; // Will be transitioned later
	imageCreateInfo.flags = 0; // No special flags for now
	if (info.type == ETextureType::TEX_CUBE)
	{
		imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VkImage imageVk;
	VK_CHECK_RESULT(vkCreateImage(_device, &imageCreateInfo, nullptr, &imageVk));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_device, imageVk, &memoryRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkDeviceMemory imageMemory;
	VK_CHECK_RESULT(vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory));
	VK_CHECK_RESULT(vkBindImageMemory(_device, imageVk, imageMemory, 0));

	bool hasData = ((image != nullptr) && (info.byteSize > 0));
	if (hasData)
	{
		VkBuffer stagingBuffer;
		VkBufferCreateInfo stagingBufferCreateInfo{};
		VkMemoryRequirements stagingMemReq;

		stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferCreateInfo.size = info.byteSize;
		stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		vkCreateBuffer(_device, &stagingBufferCreateInfo, nullptr, &stagingBuffer);
		vkGetBufferMemoryRequirements(_device, stagingBuffer, &stagingMemReq);
		VkMemoryAllocateInfo stagingAllocInfo{};
		stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		stagingAllocInfo.allocationSize = stagingMemReq.size;
		stagingAllocInfo.memoryTypeIndex = getMemoryTypeIndex(stagingMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VkDeviceMemory stagingMemory;
		VK_CHECK_RESULT(vkAllocateMemory(_device, &stagingAllocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(_device, stagingBuffer, stagingMemory, 0));

		uint8* mappedData;
		VK_CHECK_RESULT(vkMapMemory(_device, stagingMemory, 0, stagingBufferCreateInfo.size, 0, reinterpret_cast<void**>(&mappedData)));
		::memcpy(mappedData, image, info.byteSize);
		vkUnmapMemory(_device, stagingMemory);

		VkCommandBuffer copyCmd = beginSingleTimeCommands();

		VkImageSubresourceRange subresourceRange = {};
		// Image only contains color data
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// Start at first mip level
		subresourceRange.baseMipLevel = 0;
		// We will transition on all mip levels
		subresourceRange.levelCount = imageCreateInfo.mipLevels;
		// The 2D texture only has one layer
		subresourceRange.layerCount = 1;

		// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = imageVk;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
		// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
		// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		vkCmdPipelineBarrier(
			copyCmd,
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			imageVk,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data());

		// Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		// TODO: 항상 SHADER_READ_ONLY_OPTIMAL로 하면 안 됨.
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
		// Source pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
		vkCmdPipelineBarrier(
			copyCmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		endSingleTimeCommands(copyCmd);

		// Clean up staging resources
		vkFreeMemory(_device, stagingMemory, nullptr);
		vkDestroyBuffer(_device, stagingBuffer, nullptr);

		initialLayout = imageMemoryBarrier.newLayout;
	}
	//else if(info.isSwapchainTexture)
	//{
	//	VkCommandBuffer copyCmd = beginSingleTimeCommands();
	//}

	VkImageAspectFlagBits aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (info.isDepthStencilBuffer)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	VkImageViewCreateInfo viewCreateInfo{};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = imageVk;
	viewCreateInfo.viewType = RHIUtilityVulkan::ToImageViewType(info.type);
	viewCreateInfo.format = imageCreateInfo.format;
	viewCreateInfo.subresourceRange.aspectMask = aspectMask;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.subresourceRange.levelCount = 1; //TODO

	VkImageView imageViewVk;
	VK_CHECK_RESULT(vkCreateImageView(_device, &viewCreateInfo, nullptr, &imageViewVk));

	TextureVulkan* textureVK = new TextureVulkan(name, info);
	textureVK->handle = imageVk;
	textureVK->imageViewVk = imageViewVk;
	textureVK->memoryVk = imageMemory;
	textureVK->layoutVk = initialLayout;

	setDebugObjectName(VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64>(imageVk), name);
	setDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, reinterpret_cast<uint64>(imageViewVk), name);

	return static_cast<RHITexture*>(textureVK);
}

void VulkanContext::DestroyTexture(RHITexture* texture)
{
	// Destroy the Vulkan texture
	TextureVulkan* textureVK = static_cast<TextureVulkan*>(texture);

	if (textureVK->imageViewVk != VK_NULL_HANDLE)
	{
		vkDestroyImageView(_device, textureVK->imageViewVk, nullptr);
		textureVK->imageViewVk = VK_NULL_HANDLE;
	}
	if (textureVK->handle != VK_NULL_HANDLE)
	{
		vkDestroyImage(_device, textureVK->handle, nullptr);
		textureVK->handle = VK_NULL_HANDLE;
	}
	if (textureVK->memoryVk != VK_NULL_HANDLE)
	{
		vkFreeMemory(_device, textureVK->memoryVk, nullptr);
		textureVK->memoryVk = VK_NULL_HANDLE;
	}
	delete textureVK;
}

RHISampler* VulkanContext::CreateSampler(const char* name, const SamplerInfo& info)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = RHIUtilityVulkan::ToFilter(info.magFilter);
	samplerInfo.minFilter = RHIUtilityVulkan::ToFilter(info.minFilter);
	samplerInfo.mipmapMode = RHIUtilityVulkan::ToMipmapMode(info.mipmapMode);
	samplerInfo.addressModeU = RHIUtilityVulkan::ToAddressMode(info.addressU);
	samplerInfo.addressModeV = RHIUtilityVulkan::ToAddressMode(info.addressV);
	samplerInfo.addressModeW = RHIUtilityVulkan::ToAddressMode(info.addressW);
	samplerInfo.anisotropyEnable = VK_FALSE; // TODO: Add anisotropy support
	samplerInfo.maxAnisotropy = 1.0f; // TODO: Set max anisotropy
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = info.isPixelCoordinate ? VK_TRUE : VK_FALSE;

	VkSampler vkSampler;
	vkCreateSampler(_device, &samplerInfo, nullptr, &vkSampler);

	SamplerVulkan* samplerVK = new SamplerVulkan(name, info);
	samplerVK->handle = vkSampler;

	setDebugObjectName(VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64>(vkSampler), name);

	return static_cast<RHISampler*>(samplerVK);
}

void VulkanContext::DestroySampler(RHISampler* sampler)
{
	SamplerVulkan* samplerVK = static_cast<SamplerVulkan*>(sampler);
	if (samplerVK->handle)
	{
		vkDestroySampler(_device, samplerVK->handle, nullptr);
		samplerVK->handle = VK_NULL_HANDLE;
	}

	delete samplerVK;
}

RHIResourceLayout* VulkanContext::CreateResourceLayout(const char* name, ResourceBinding* bindings, uint32 bindingCount)
{
	ResourceLayoutVulkan* resourceLayoutVK = new ResourceLayoutVulkan(name, bindings, bindingCount);
	std::vector<VkDescriptorSetLayoutBinding>& bindingVks = resourceLayoutVK->bindingVks;
	bindingVks.resize(bindingCount);

	for (uint32 i = 0; i < bindingCount; i++)
	{
		VkDescriptorSetLayoutBinding& binding = bindingVks[i];
		VkDescriptorType descType;
		switch (bindings[i].type)
		{
		case EResourceType::SAMPLER:
			descType = VK_DESCRIPTOR_TYPE_SAMPLER;
			break;
		case EResourceType::COMBINED_IMAGE_SAMPLER:
			descType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		case EResourceType::SAMPLED_IMAGE:
			descType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			break;
		case EResourceType::STORAGE_IMAGE:
			descType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			break;
		case EResourceType::UNIFORM_BUFFER:
			descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		case EResourceType::STORAGE_BUFFER:
			descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			break;
		case EResourceType::STORAGE_BUFFER_DYNAMIC:
			descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			break;
		case EResourceType::UNIFORM_BUFFER_DYNAMIC:
			descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			break;
		case EResourceType::INPUT_ATTACHMENT:
			descType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			break;
		default:
			HS_LOG(crash, "Unsupported resource type: %d", static_cast<int>(bindings[i].type));
			return nullptr; // Unsupported resource type
		}

		binding.binding = bindings[i].binding;
		binding.descriptorCount = bindings[i].arrayCount;
		binding.descriptorType = descType;
		binding.pImmutableSamplers = nullptr;
		binding.stageFlags = RHIUtilityVulkan::ToShaderStageFlags(bindings[i].stage);
	}

	VkDescriptorSetLayout layout;
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindingCount;
	layoutInfo.pBindings = bindingVks.data();
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &layout));

	resourceLayoutVK->handle = layout;

	setDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, reinterpret_cast<uint64>(layout), name);

	return static_cast<RHIResourceLayout*>(resourceLayoutVK);
}

void VulkanContext::DestroyResourceLayout(RHIResourceLayout* resourceLayout)
{
	// Destroy the Vulkan resource layout

	ResourceLayoutVulkan* resourceLayoutVK = static_cast<ResourceLayoutVulkan*>(resourceLayout);
	if (resourceLayoutVK->handle != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(_device, resourceLayoutVK->handle, nullptr);
		resourceLayoutVK->handle = VK_NULL_HANDLE;
	}
}

RHIResourceSet* VulkanContext::CreateResourceSet(const char* name, RHIResourceLayout* resourceLayouts)
{
	// Create a Vulkan resource set
	VkDescriptorSet rSetVk = _descriptorPoolAllocator.AllocateDescriptorSet(static_cast<ResourceLayoutVulkan*>(resourceLayouts)->handle, nullptr);

	if (rSetVk == VK_NULL_HANDLE)
	{
		return nullptr;
	}
	ResourceSetVulkan* resourceSetVK = new ResourceSetVulkan(name);
	resourceSetVK->handle = rSetVk;
	resourceSetVK->layoutVK = static_cast<ResourceLayoutVulkan*>(resourceLayouts);

	setDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, reinterpret_cast<uint64>(rSetVk), name);

	return static_cast<RHIResourceSet*>(resourceSetVK);
}

void VulkanContext::DestroyResourceSet(RHIResourceSet* resourceSet)
{
	// Destroy the Vulkan resource set
	ResourceSetVulkan* resourceSetVK = static_cast<ResourceSetVulkan*>(resourceSet);
	if (resourceSetVK->handle != VK_NULL_HANDLE)
	{
		_descriptorPoolAllocator.FreeDescriptorSet(resourceSetVK->handle);
		resourceSetVK->handle = VK_NULL_HANDLE;
	}
	delete resourceSetVK;
}

RHIResourceSetPool* VulkanContext::CreateResourceSetPool(const char* name, uint32 bufferSize, uint32 textureSize)
{
	VkDescriptorPoolSize poolSizes[]{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, bufferSize },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, bufferSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, textureSize },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureSize },
		{ VK_DESCRIPTOR_TYPE_SAMPLER, textureSize },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, textureSize},
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, textureSize }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 512;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);
	poolInfo.pNext = nullptr;

	VkDescriptorPool descriptorPool;
	VK_CHECK_RESULT(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &descriptorPool));
	ResourceSetPoolVulkan* resourceSetPoolVK = new ResourceSetPoolVulkan(name);
	resourceSetPoolVK->handle = descriptorPool;

	setDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_POOL, reinterpret_cast<uint64>(descriptorPool), name);

	return static_cast<RHIResourceSetPool*>(resourceSetPoolVK);
}

void VulkanContext::DestroyResourceSetPool(RHIResourceSetPool* resourceSetPool)
{
	// Destroy the Vulkan resource set pool
	ResourceSetPoolVulkan* resourceSetPoolVK = static_cast<ResourceSetPoolVulkan*>(resourceSetPool);
	if (resourceSetPoolVK->handle != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(_device, resourceSetPoolVK->handle, nullptr);
		resourceSetPoolVK->handle = VK_NULL_HANDLE;
	}
	delete resourceSetPoolVK;
}

RHICommandPool* VulkanContext::CreateCommandPool(const char* name, uint32 queueFamilyIndex)
{
	// Create a Vulkan command pool
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.queueFamilyIndex = queueFamilyIndex;

	VkCommandPool commandPool;
	vkCreateCommandPool(_device, &poolInfo, nullptr, &commandPool);

	CommandPoolVulkan* commandPoolVK = new CommandPoolVulkan(name);
	commandPoolVK->handle = commandPool;

	setDebugObjectName(VK_OBJECT_TYPE_COMMAND_POOL, reinterpret_cast<uint64>(commandPool), name);

	return static_cast<RHICommandPool*>(commandPoolVK);
}

void VulkanContext::DestroyCommandPool(RHICommandPool* commandPool)
{
	// Destroy the Vulkan command pool
	CommandPoolVulkan* commandPoolVK = static_cast<CommandPoolVulkan*>(commandPool);
	if (commandPoolVK->handle != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(_device, commandPoolVK->handle, nullptr);
		commandPoolVK->handle = VK_NULL_HANDLE;
	}

	delete commandPoolVK;
}

RHICommandBuffer* VulkanContext::CreateCommandBuffer(const char* name)
{
	// Create a Vulkan command buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _defaultCommandPool;
	allocInfo.commandBufferCount = 1;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer cmdBufferVk;
	vkAllocateCommandBuffers(_device, &allocInfo, &cmdBufferVk);

	CommandBufferVulkan* commandBufferVK = new CommandBufferVulkan(name);
	commandBufferVK->handle = cmdBufferVk;

	setDebugObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, reinterpret_cast<uint64>(cmdBufferVk), name);

	return static_cast<RHICommandBuffer*>(commandBufferVK);
}

void VulkanContext::DestroyCommandBuffer(RHICommandBuffer* commandBuffer)
{
	// Destroy the Vulkan command buffer
	CommandBufferVulkan* commandBufferVK = static_cast<CommandBufferVulkan*>(commandBuffer);
	if (commandBufferVK->handle)
	{
		vkFreeCommandBuffers(_device, _defaultCommandPool, 1, &commandBufferVK->handle);
		commandBufferVK->handle = VK_NULL_HANDLE;
	}
	delete commandBuffer;
}

void VulkanContext::Submit(Swapchain* swapchain, RHICommandBuffer** buffers, size_t bufferCount)
{
	static std::vector<VkCommandBuffer> commandBufferVks;

	HS_ASSERT(swapchain != nullptr, "Swapchain is null in VulkanContext::Submit");
	HS_ASSERT(bufferCount > 0, "Buffer count must be greater than 0 in VulkanContext::Submit");

	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &swapchainVK->syncObjects.imageAvailableSemaphores[swapchainVK->_frameIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &swapchainVK->syncObjects.renderFinishedSemaphores[swapchainVK->_curImageIndex];

	if (commandBufferVks.size() < bufferCount)
	{
		commandBufferVks.resize(bufferCount);
	}

	for (size_t i = 0; i < bufferCount; ++i)
	{
		CommandBufferVulkan* commandBufferVK = static_cast<CommandBufferVulkan*>(buffers[i]);
		commandBufferVks[i] = commandBufferVK->handle;
	}
	submitInfo.pCommandBuffers = commandBufferVks.data();
	submitInfo.commandBufferCount = static_cast<uint32_t>(bufferCount);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStages;

	VK_CHECK_RESULT(vkQueueSubmit(_device.graphicsQueue, 1, &submitInfo, swapchainVK->syncObjects.inFlightFences[swapchainVK->_frameIndex]));
	//vkQueueWaitIdle(_device.graphicsQueue); // TEST: Wait for the queue to finish processing the submitted commands
}

void VulkanContext::Present(Swapchain* swapchain)
{
	HS_ASSERT(swapchain != nullptr, "Swapchain is null in VulkanContext::Present");
	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &(swapchainVK->handle);
	presentInfo.pImageIndices = &(swapchainVK->_curImageIndex);
	presentInfo.pWaitSemaphores = &(swapchainVK->syncObjects.renderFinishedSemaphores[swapchainVK->_curImageIndex]);
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pResults = nullptr; // 스왑체인 하나만 쓸 땐 필요없다.

	VkResult result = vkQueuePresentKHR(_device.graphicsQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		HS_LOG(warning, "Swapchain is out of date or suboptimal during present.");
	}
	else if (result != VK_SUCCESS)
	{
		HS_LOG(error, "Failed to present swapchain image: %d", result);
	}
}

void VulkanContext::WaitForIdle() const
{
	// Wait for the Vulkan device to be idle
	vkDeviceWaitIdle(_device.logicalDevice);
}

void VulkanContext::cleanup()
{
	if (s_enableValidationLayers)
	{
		destroyDebugUtilsMessengerEXT(_instanceVk, _debugMessenger, nullptr);
	}
	delete _device;

	vkDestroyInstance(_instanceVk, nullptr);
}
#pragma region >>> Implementation create functions

bool VulkanContext::createInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "HSMR";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
	appInfo.pEngineName = "HSMR";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	uint32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	std::vector<const char*> extensionNames;
	for (const auto& extension : availableExtensions)
	{
		HS_LOG(info, "Available Extension: %s", extension.extensionName);
		if (strcmp(extension.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			extensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		}
		else if (strcmp(extension.extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			extensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		}
		else if (strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
		{
			extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
	}

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = extensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
	instanceCreateInfo.pNext = nullptr;

	uint32 layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	bool isLayerFound = false;
	for (const char* validationLayer : s_validationLayers)
	{
		for (const auto& layer : availableLayers)
		{
			if (strcmp(validationLayer, layer.layerName) == 0)
			{
				isLayerFound = true;
				break;
			}
		}
	}

	bool useValidationLayers = s_enableValidationLayers && isLayerFound;
	if (useValidationLayers)
	{
		instanceCreateInfo.enabledLayerCount = s_validationLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = s_validationLayers.data();
	}

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (useValidationLayers)
	{
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.flags = 0;
		debugCreateInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/ VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = hs_rhi_vk_report_debug_callback;
		debugCreateInfo.pUserData = nullptr;
		debugCreateInfo.pNext = instanceCreateInfo.pNext;

		instanceCreateInfo.pNext = &debugCreateInfo;
	}

	VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &_instanceVk));

	VK_CHECK_RESULT(createDebugUtilsMessengerEXT(_instanceVk, &debugCreateInfo, &_debugMessenger, nullptr));

	return true;
}

void VulkanContext::createDefaultCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Command buffers can be reset individually
	poolInfo.queueFamilyIndex = _device.queueFamilyIndices.graphics;
	VK_CHECK_RESULT(vkCreateCommandPool(_device, &poolInfo, nullptr, &_defaultCommandPool));
}

VkSurfaceKHR VulkanContext::createSurface(const NativeWindow& nativeWindow)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)GetModuleHandleW(NULL);
	surfaceCreateInfo.hwnd = (HWND)nativeWindow.handle;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(_instanceVk, &surfaceCreateInfo, nullptr, &surface));

	return surface;
}

VkRenderPass VulkanContext::createRenderPass(const RenderPassInfo& info)
{
	auto attachmentCount = info.colorAttachmentCount + static_cast<uint8>(info.useDepthStencilAttachment);

	VkImageLayout colorFinalLayout = info.isSwapchainRenderPass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkImageLayout depthStencilFinalLayout = info.isSwapchainRenderPass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	std::vector<VkAttachmentDescription> attachments(attachmentCount);
	int index = 0;
	for (; index < info.colorAttachmentCount; index++)
	{
		VkAttachmentLoadOp loadOp = RHIUtilityVulkan::ToLoadOp(info.colorAttachments[index].loadAction);
		VkAttachmentStoreOp storeOp = RHIUtilityVulkan::ToStoreOp(info.colorAttachments[index].storeAction);
		VkSampleCountFlagBits sampleCount = static_cast<VkSampleCountFlagBits>(info.colorAttachments[index].sampleCount);

		attachments[index].flags = 0;
		attachments[index].format = RHIUtilityVulkan::ToPixelFormat(info.colorAttachments[index].format);
		attachments[index].loadOp = loadOp;
		attachments[index].storeOp = storeOp;
		attachments[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // No stencil attachment
		attachments[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // No stencil attachment
		attachments[index].initialLayout = (loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[index].finalLayout = colorFinalLayout; // Final layout for color attachments
		attachments[index].samples = VK_SAMPLE_COUNT_1_BIT; // TOOD: Multisampling
	}

	// TODO: ResolveColorAttachments

	if (info.useDepthStencilAttachment)
	{
		VkAttachmentLoadOp depthLoadOp = RHIUtilityVulkan::ToLoadOp(info.depthStencilAttachment.loadAction);
		VkAttachmentStoreOp depthStoreOp = RHIUtilityVulkan::ToStoreOp(info.depthStencilAttachment.storeAction);
		VkSampleCountFlagBits sampleCount = static_cast<VkSampleCountFlagBits>(info.depthStencilAttachment.sampleCount);
		attachments[index].flags = 0;
		attachments[index].format = RHIUtilityVulkan::ToPixelFormat(info.depthStencilAttachment.format);
		attachments[index].loadOp = depthLoadOp;
		attachments[index].storeOp = depthStoreOp;
		attachments[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // No stencil attachment
		attachments[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // No stencil attachment
		attachments[index].initialLayout = (depthLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[index].finalLayout = depthStencilFinalLayout; // Final layout for depth/stencil attachments
		attachments[index].samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Multisampling
		index++;
	}

	VkSubpassDescription subPass{};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPass.colorAttachmentCount = info.colorAttachmentCount;
	std::vector<VkAttachmentReference> colorAttachments(info.colorAttachmentCount);
	for (uint32 i = 0; i < info.colorAttachmentCount; i++)
	{
		colorAttachments[i].attachment = i;
		colorAttachments[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	subPass.pColorAttachments = colorAttachments.data();

	VkAttachmentReference depthStencilAttachmentRef{};
	if (info.useDepthStencilAttachment)
	{
		depthStencilAttachmentRef.attachment = index - 1; // Last attachment is depth/stencil
		depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		subPass.pDepthStencilAttachment = &depthStencilAttachmentRef;
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.pNext = nullptr;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPass;
	renderPassInfo.dependencyCount = 0;

	VkRenderPass renderPassVk;
	vkCreateRenderPass(_device, &renderPassInfo, nullptr, &renderPassVk);

	return renderPassVk; // Placeholder, implement the rest of the function
}

VkFramebuffer VulkanContext::createFramebuffer(const FramebufferInfo& info)
{
	RenderPassVulkan* renderPassVK = static_cast<RenderPassVulkan*>(info.renderPass);

	std::vector<VkImageView> attachments;
	attachments.reserve(info.colorBuffers.size() + (info.depthStencilBuffer ? 1 : 0));
	for (const auto& colorBuffer : info.colorBuffers)
	{
		TextureVulkan* textureVK = static_cast<TextureVulkan*>(colorBuffer);
		attachments.push_back(textureVK->imageViewVk);

	}

	if (info.depthStencilBuffer)
	{
		TextureVulkan* depthTextureVK = static_cast<TextureVulkan*>(info.depthStencilBuffer);
		if (depthTextureVK && depthTextureVK->imageViewVk)
		{
			attachments.push_back(depthTextureVK->imageViewVk);
		}
	}

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPassVK->handle;
	framebufferInfo.attachmentCount = static_cast<uint32>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = info.width;
	framebufferInfo.height = info.height;
	framebufferInfo.layers = 1;

	VkFramebuffer framebufferVk;
	VK_CHECK_RESULT(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &framebufferVk));

	return framebufferVk;
}

VkPipeline VulkanContext::createGraphicsPipeline(const GraphicsPipelineInfo& info)
{
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	// TODO: ResourceLayout 이용하기
	// Pipeline Layout 
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	VkPipelineLayout layout;
	vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &layout);

	pipelineCreateInfo.layout = layout;

	//ShaderStage
	uint32 stageCount = static_cast<uint32>(info.shaderDesc.stages.size());
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(stageCount);
	for (size_t i = 0; i < stageCount; i++)
	{
		auto* shader = info.shaderDesc.stages[i];
		shaderStages[i] = static_cast<ShaderVulkan*>(shader)->stageInfo;
	}

	pipelineCreateInfo.stageCount = stageCount;
	pipelineCreateInfo.pStages = shaderStages.data();

	//Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.primitiveRestartEnable = info.inputAssemblyDesc.isRestartEnable ? VK_TRUE : VK_FALSE; // Primitive restart disabled
	inputAssembly.topology = RHIUtilityVulkan::ToPrimitiveTopology(info.inputAssemblyDesc.primitiveTopology);

	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;

	// Vertex Input
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// Bindings
	uint32 bindingCount = info.vertexInputDesc.layouts.size();
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(bindingCount);
	for (size_t i = 0; i < bindingCount; i++)
	{
		const auto& layout = info.vertexInputDesc.layouts[i];
		auto& bindingDesc = bindingDescriptions[i];
		bindingDesc.binding = layout.binding;
		bindingDesc.stride = layout.stride;
		bindingDesc.inputRate = layout.useInstancing ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
	}

	// Attributes
	uint32 attributeCount = info.vertexInputDesc.attributes.size();
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(attributeCount);
	for (size_t i = 0; i < attributeCount; i++)
	{
		const auto& attribute = info.vertexInputDesc.attributes[i];
		auto& attributeDesc = attributeDescriptions[i];
		attributeDesc.binding = attribute.binding;
		attributeDesc.location = attribute.location;
		attributeDesc.format = RHIUtilityVulkan::ToVertexFormat(attribute.format);
		attributeDesc.offset = attribute.offset;
	}

	vertexInputInfo.vertexBindingDescriptionCount = bindingCount;
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attributeCount;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;

	// Tesellation State
	{
		// TODO
	}

	// Rasterizer State
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = info.rasterizerDesc.depthClampEnable ? VK_TRUE : VK_FALSE;
	rasterizer.rasterizerDiscardEnable = info.rasterizerDesc.rasterizerDiscardEnable ? VK_TRUE : VK_FALSE;
	rasterizer.polygonMode = RHIUtilityVulkan::ToPolygonMode(info.rasterizerDesc.polygonMode);
	rasterizer.lineWidth = info.rasterizerDesc.lineWidth;
	rasterizer.cullMode = RHIUtilityVulkan::ToCullMode(info.rasterizerDesc.cullMode);
	rasterizer.frontFace = RHIUtilityVulkan::ToFrontFace(info.rasterizerDesc.frontFace);
	rasterizer.depthBiasEnable = info.rasterizerDesc.depthBiasEnable ? VK_TRUE : VK_FALSE;
	rasterizer.depthBiasConstantFactor = info.rasterizerDesc.depthBiasConstant;
	rasterizer.depthBiasSlopeFactor = info.rasterizerDesc.depthBiasSlope;
	rasterizer.depthBiasClamp = info.rasterizerDesc.depthBiasClamp;

	pipelineCreateInfo.pRasterizationState = &rasterizer;


	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pNext = nullptr;
	multisampleState.flags = 0;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // TODO: MSAA
	multisampleState.sampleShadingEnable = VK_FALSE; // No sample shading
	multisampleState.minSampleShading = 1.0f; // No sample shading

	pipelineCreateInfo.pMultisampleState = &multisampleState;

	// DepthStencil
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = info.depthStencilDesc.depthTestEnable ? VK_TRUE : VK_FALSE;
	depthStencil.depthWriteEnable = info.depthStencilDesc.depthWriteEnable ? VK_TRUE : VK_FALSE;
	depthStencil.depthCompareOp = RHIUtilityVulkan::ToCompareOp(info.depthStencilDesc.depthCompareOp);
	depthStencil.depthBoundsTestEnable = info.depthStencilDesc.depthBoundTestEnable ? VK_TRUE : VK_FALSE;
	depthStencil.stencilTestEnable = info.depthStencilDesc.stencilTestEnable ? VK_TRUE : VK_FALSE;
	depthStencil.front.failOp = RHIUtilityVulkan::ToStencilOp(info.depthStencilDesc.stencilFront.failOp);
	depthStencil.front.passOp = RHIUtilityVulkan::ToStencilOp(info.depthStencilDesc.stencilFront.passOp);
	depthStencil.front.depthFailOp = RHIUtilityVulkan::ToStencilOp(info.depthStencilDesc.stencilFront.depthFailOp);
	depthStencil.front.compareOp = RHIUtilityVulkan::ToCompareOp(info.depthStencilDesc.stencilFront.compareOp);
	depthStencil.front.compareMask = info.depthStencilDesc.stencilFront.compareMask;
	depthStencil.front.writeMask = info.depthStencilDesc.stencilFront.writeMask;
	depthStencil.front.reference = info.depthStencilDesc.stencilFront.reference;
	depthStencil.back.failOp = RHIUtilityVulkan::ToStencilOp(info.depthStencilDesc.stencilBack.failOp);
	depthStencil.back.passOp = RHIUtilityVulkan::ToStencilOp(info.depthStencilDesc.stencilBack.passOp);
	depthStencil.back.depthFailOp = RHIUtilityVulkan::ToStencilOp(info.depthStencilDesc.stencilBack.depthFailOp);
	depthStencil.back.compareOp = RHIUtilityVulkan::ToCompareOp(info.depthStencilDesc.stencilBack.compareOp);
	depthStencil.back.compareMask = info.depthStencilDesc.stencilBack.compareMask;
	depthStencil.back.writeMask = info.depthStencilDesc.stencilBack.writeMask;
	depthStencil.back.reference = info.depthStencilDesc.stencilBack.reference;
	depthStencil.minDepthBounds = info.depthStencilDesc.minDepthBound;
	depthStencil.maxDepthBounds = info.depthStencilDesc.maxDepthBound;
	depthStencil.pNext = nullptr;

	pipelineCreateInfo.pDepthStencilState = &depthStencil;

	// ColorBlend
	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = static_cast<uint32>(info.colorBlendDesc.attachmentCount);
	colorBlendState.logicOpEnable = info.colorBlendDesc.logicOpEnable;
	colorBlendState.logicOp = RHIUtilityVulkan::ToLogicOp(info.colorBlendDesc.blendLogic);
	colorBlendState.blendConstants[0] = info.colorBlendDesc.blendConstants[0];
	colorBlendState.blendConstants[1] = info.colorBlendDesc.blendConstants[1];
	colorBlendState.blendConstants[2] = info.colorBlendDesc.blendConstants[2];
	colorBlendState.blendConstants[3] = info.colorBlendDesc.blendConstants[3];

	std::vector<VkPipelineColorBlendAttachmentState> attachmentVks(info.colorBlendDesc.attachmentCount);
	for (size_t i = 0; i < info.colorBlendDesc.attachmentCount; i++)
	{
		const auto& attachment = info.colorBlendDesc.attachments[i];
		attachmentVks[i].alphaBlendOp = RHIUtilityVulkan::ToBlendOp(attachment.alphaBlendOp);
		attachmentVks[i].colorBlendOp = RHIUtilityVulkan::ToBlendOp(attachment.colorBlendOp);
		attachmentVks[i].srcAlphaBlendFactor = RHIUtilityVulkan::ToBlendFactor(attachment.srcAlphaFactor);
		attachmentVks[i].dstAlphaBlendFactor = RHIUtilityVulkan::ToBlendFactor(attachment.dstAlphaFactor);
		attachmentVks[i].srcColorBlendFactor = RHIUtilityVulkan::ToBlendFactor(attachment.srcColorFactor);
		attachmentVks[i].dstColorBlendFactor = RHIUtilityVulkan::ToBlendFactor(attachment.dstColorFactor);
		attachmentVks[i].blendEnable = attachment.blendEnable ? VK_TRUE : VK_FALSE;
		attachmentVks[i].colorWriteMask = attachment.writeMask;
	}
	colorBlendState.pAttachments = attachmentVks.data();
	pipelineCreateInfo.pColorBlendState = &colorBlendState;


	VkViewport viewport{};

	viewport.x = 0;
	viewport.y = 0;

	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	viewportInfo.scissorCount = 1;

	pipelineCreateInfo.pViewportState = &viewportInfo;

	// Dynamic State
	VkDynamicState dynamicStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	dynamicState.pNext = nullptr;

	pipelineCreateInfo.pDynamicState = &dynamicState;

	pipelineCreateInfo.renderPass = static_cast<RenderPassVulkan*>(info.renderPass)->handle;

	VkPipeline pipelineVk;
	vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipelineVk);

	return pipelineVk;
}

VkPipeline VulkanContext::createComputePipeline(const ComputePipelineInfo& info)
{
	return VK_NULL_HANDLE;

}

#pragma endregion 

#pragma region >>> Resource Utility Functions


uint32 VulkanContext::getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(_device.physicalDevice, &deviceMemoryProperties);
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}
	return 0;
}

#pragma endregion

#pragma region >>> Command Utility Functions

VkCommandBuffer VulkanContext::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _defaultCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBufferVk;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(_device, &allocInfo, &commandBufferVk));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // One-time submit

	vkBeginCommandBuffer(commandBufferVk, &beginInfo);

	return commandBufferVk;
}

void VulkanContext::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(_device.transferQueue, 1, &submitInfo, VK_NULL_HANDLE));
	vkQueueWaitIdle(_device.transferQueue);

	vkFreeCommandBuffers(_device, _defaultCommandPool, 1, &commandBuffer);
}

void VulkanContext::traisitionImageLayout(
	VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout
)
{
	VkCommandBuffer cmdBufferVk = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.pNext = nullptr;

	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Assuming color image
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;


	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		HS_LOG(error, "Unsupported layout transition!");
		endSingleTimeCommands(cmdBufferVk);
		return;
	}

	vkCmdPipelineBarrier(cmdBufferVk,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(cmdBufferVk);
}

void VulkanContext::copyBufferToImage(
	VkBuffer buffer,
	VkImage image,
	uint32 width,
	uint32 height
)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Assuming color image
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(commandBuffer, buffer, image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

#pragma endregion


#pragma region >>> Debugging Functions

HS_FORCEINLINE void VulkanContext::setDebugObjectName(VkObjectType type, uint64 handle, const char* name)
{
#ifdef _DEBUG
	static auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(_instanceVk, "vkSetDebugUtilsObjectNameEXT");
	HS_ASSERT(vkSetDebugUtilsObjectNameEXT, "function addr is nullptr");

	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = type;
	nameInfo.objectHandle = handle;
	nameInfo.pObjectName = name;
	nameInfo.pNext = nullptr;

	vkSetDebugUtilsObjectNameEXT(_device, &nameInfo);
#endif
}

VkResult VulkanContext::createDebugUtilsMessengerEXT
(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	VkDebugUtilsMessengerEXT* pDebugMessenger,
	const VkAllocationCallbacks* npAllocator = nullptr
)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, npAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanContext::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* npAllocator = nullptr)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, pDebugMessenger, npAllocator);
	}
}

#pragma endregion

HS_NS_END