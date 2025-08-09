//
//  RHIContextVulkan_Claude.cpp
//  Engine
//
//  Created by Claude for HSMR
//
#include "Engine/RHI/Vulkan/RHIContextVulkan.h"

#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"
#include "Engine/RHI/Vulkan/RenderHandleVulkan.h"
#include "Engine/RHI/Vulkan/ResourceHandleVulkan.h"
#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"
#include "Engine/RHI/Vulkan/SwapchainVulkan.h"

#include "Engine/Core/Window.h"
#include "Engine/Core/FileSystem.h"
#include "Engine/Core/EngineContext.h"

#include "Engine/Platform/Windows/PlatformApplicationWindows.h"

#include <algorithm>
#include <set>
#include <array>
#include <fstream>

// Validation layers
static const std::vector<const char*> s_validationLayers =
{
#ifdef _DEBUG
	"VK_LAYER_KHRONOS_validation",
#endif
};

#ifdef _DEBUG
static constexpr bool s_enableValidationLayers = true;
#else
static constexpr bool s_enableValidationLayers = false;
#endif

// Debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL hs_rhi_vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	const char* prefix = "";
	ELogLevel logLevel = ELogLevel::INFO;

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		prefix = "ERROR";
		logLevel = ELogLevel::ERROR;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		prefix = "WARNING";
		logLevel = ELogLevel::WARNING;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		prefix = "INFO";
		logLevel = ELogLevel::INFO;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		prefix = "VERBOSE";
		logLevel = ELogLevel::DEBUG;
	}

	// Filter out some verbose messages
	if (pCallbackData->pMessageIdName)
	{
		std::string msgId(pCallbackData->pMessageIdName);
		// Filter out some common verbose messages
		if (msgId.find("BestPractices") != std::string::npos && logLevel == ELogLevel::INFO)
		{
			return VK_FALSE;
		}
	}

	HS_LOG(logLevel, "%s: [%s] Code %i : %s", 
		prefix, 
		pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "Unknown",
		pCallbackData->messageIdNumber, 
		pCallbackData->pMessage);

	return VK_FALSE;
}

HS_NS_BEGIN

// Memory management helper
class VulkanMemoryAllocator
{
public:
	struct MemoryTypeInfo
	{
		uint32 typeIndex;
		VkMemoryPropertyFlags properties;
	};

	static uint32 FindMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32 i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		HS_LOG(error, "Failed to find suitable memory type!");
		return UINT32_MAX;
	}

	static VkDeviceMemory AllocateMemory(VkDevice device, VkPhysicalDevice physicalDevice, 
		VkMemoryRequirements memReqs, VkMemoryPropertyFlags properties)
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memReqs.memoryTypeBits, properties);

		VkDeviceMemory memory;
		if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		{
			return VK_NULL_HANDLE;
		}

		return memory;
	}
};

// Implementation of RHIContextVulkan with complete functionality
class RHIContextVulkanImpl : public RHIContextVulkan
{
public:
	RHIContextVulkanImpl() = default;
	~RHIContextVulkanImpl() override;

	bool Initialize() override;
	void Finalize() override;

	void Suspend(Swapchain* swapchain) override;
	void Restore(Swapchain* swapchain) override;

	uint32 AcquireNextImage(Swapchain* swapchain) override;

	Swapchain* CreateSwapchain(SwapchainInfo info) override;
	void DestroySwapchain(Swapchain* swapchain) override;

	RenderPass* CreateRenderPass(const RenderPassInfo& info) override;
	void DestroyRenderPass(RenderPass* renderPass) override;

	Framebuffer* CreateFramebuffer(const FramebufferInfo& info) override;
	void DestroyFramebuffer(Framebuffer* framebuffer) override;

	GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineInfo& info) override;
	void DestroyGraphicsPipeline(GraphicsPipeline* pipeline) override;

	Shader* CreateShader(const ShaderInfo& info, const char* path) override;
	Shader* CreateShader(const ShaderInfo& info, const char* byteCode, size_t byteCodeSize) override;
	void DestroyShader(Shader* shader) override;

	Buffer* CreateBuffer(const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption) override;
	Buffer* CreateBuffer(const void* data, size_t dataSize, const BufferInfo& info) override;
	void DestroyBuffer(Buffer* buffer) override;

	Texture* CreateTexture(void* image, const TextureInfo& info) override;
	Texture* CreateTexture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage) override;
	void DestroyTexture(Texture* texture) override;

	Sampler* CreateSampler(const SamplerInfo& info) override;
	void DestroySampler(Sampler* sampler) override;

	ResourceLayout* CreateResourceLayout() override;
	void DestroyResourceLayout(ResourceLayout* resourceLayout) override;

	ResourceSet* CreateResourceSet() override;
	void DestroyResourceSet(ResourceSet* resourceSet) override;

	ResourceSetPool* CreateResourceSetPool() override;
	void DestroyResourceSetPool(ResourceSetPool* resourceSetPool) override;

	CommandPool* CreateCommandPool(uint32 queueFamilyIndex = 0) override;
	void DestroyCommandPool(CommandPool* cmdPool) override;

	CommandBuffer* CreateCommandBuffer() override;
	void DestroyCommandBuffer(CommandBuffer* commandBuffer) override;

	Fence* CreateFence() override;
	void DestroyFence(Fence* fence) override;

	Semaphore* CreateSemaphore() override;
	void DestroySemaphore(Semaphore* semaphore) override;

	void Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount) override;
	void Present(Swapchain* swapchain) override;

	void WaitForIdle() const override;

private:
	// Instance creation
	bool createInstance();
	bool setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();

	// Device creation helpers
	bool createDevice();
	void createDefaultPools();
	
	// Surface creation
	VkSurfaceKHR createSurface(const NativeWindow& nativeWindow);

	// Resource creation helpers
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void createImage(uint32 width, uint32 height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height);

	// Command buffer helpers
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	// Cleanup
	void cleanup();

	// Debug utilities
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);
	void setDebugObjectName(VkObjectType type, uint64 handle, const char* name);

private:
	// Core Vulkan objects
	VkInstance _instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
	RHIDeviceVulkan _device;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	
	// Default pools
	VkCommandPool _defaultCommandPool = VK_NULL_HANDLE;
	VkDescriptorPool _defaultDescriptorPool = VK_NULL_HANDLE;
	
	// Queue handles
	VkQueue _graphicsQueue = VK_NULL_HANDLE;
	VkQueue _presentQueue = VK_NULL_HANDLE;
	VkQueue _computeQueue = VK_NULL_HANDLE;
	VkQueue _transferQueue = VK_NULL_HANDLE;
};

RHIContextVulkanImpl::~RHIContextVulkanImpl()
{
	Finalize();
}

bool RHIContextVulkanImpl::Initialize()
{
	HS_LOG(info, "Initializing Vulkan RHI Context...");

	// Create Vulkan instance
	if (!createInstance())
	{
		HS_LOG(error, "Failed to create Vulkan instance");
		return false;
	}

	// Setup debug messenger
	if (s_enableValidationLayers && !setupDebugMessenger())
	{
		HS_LOG(error, "Failed to setup debug messenger");
		return false;
	}

	// Create logical device
	if (!_device.Create(_instance))
	{
		HS_LOG(error, "Failed to create logical device");
		return false;
	}

	// Store physical device handle
	_physicalDevice = _device.GetPhysicalDevice();

	// Get queue handles
	vkGetDeviceQueue(_device, _device.queueFamilyIndices.graphics, 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, _device.queueFamilyIndices.present, 0, &_presentQueue);
	
	if (_device.queueFamilyIndices.compute != UINT32_MAX)
	{
		vkGetDeviceQueue(_device, _device.queueFamilyIndices.compute, 0, &_computeQueue);
	}
	
	if (_device.queueFamilyIndices.transfer != UINT32_MAX)
	{
		vkGetDeviceQueue(_device, _device.queueFamilyIndices.transfer, 0, &_transferQueue);
	}

	// Create default pools
	createDefaultPools();

	HS_LOG(info, "Vulkan RHI Context initialized successfully");
	return true;
}

void RHIContextVulkanImpl::Finalize()
{
	if (_device != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(_device);

		// Destroy default pools
		if (_defaultDescriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(_device, _defaultDescriptorPool, nullptr);
			_defaultDescriptorPool = VK_NULL_HANDLE;
		}

		if (_defaultCommandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(_device, _defaultCommandPool, nullptr);
			_defaultCommandPool = VK_NULL_HANDLE;
		}

		// Destroy device
		_device.Destroy();
	}

	// Destroy debug messenger
	if (s_enableValidationLayers && _debugMessenger != VK_NULL_HANDLE)
	{
		destroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
		_debugMessenger = VK_NULL_HANDLE;
	}

	// Destroy instance
	if (_instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(_instance, nullptr);
		_instance = VK_NULL_HANDLE;
	}

	HS_LOG(info, "Vulkan RHI Context finalized");
}

bool RHIContextVulkanImpl::createInstance()
{
	// Check validation layer support
	if (s_enableValidationLayers && !checkValidationLayerSupport())
	{
		HS_LOG(error, "Validation layers requested, but not available!");
		return false;
	}

	// Application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "HSMR Application";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "HSMR Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	// Instance create info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Extensions
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Validation layers
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (s_enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32>(s_validationLayers.size());
		createInfo.ppEnabledLayerNames = s_validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// Create instance
	VkResult result = vkCreateInstance(&createInfo, nullptr, &_instance);
	if (result != VK_SUCCESS)
	{
		HS_LOG(error, "Failed to create Vulkan instance: %s", RHIUtilityVulkan::ToString(result));
		return false;
	}

	HS_LOG(info, "Vulkan instance created successfully");
	return true;
}

bool RHIContextVulkanImpl::setupDebugMessenger()
{
	if (!s_enableValidationLayers) return true;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (createDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
	{
		HS_LOG(error, "Failed to set up debug messenger!");
		return false;
	}

	return true;
}

void RHIContextVulkanImpl::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = hs_rhi_vk_debug_callback;
	createInfo.pUserData = nullptr;
}

std::vector<const char*> RHIContextVulkanImpl::getRequiredExtensions()
{
	std::vector<const char*> extensions;

	// Platform-specific surface extension
#ifdef _WIN32
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
	extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	// Common extensions
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

	// Debug extensions
	if (s_enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// Verify extensions are available
	uint32 availableCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(availableCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableCount, availableExtensions.data());

	HS_LOG(info, "Available Vulkan instance extensions:");
	for (const auto& ext : availableExtensions)
	{
		HS_LOG(info, "  - %s", ext.extensionName);
	}

	// Check required extensions
	for (const auto& required : extensions)
	{
		bool found = false;
		for (const auto& available : availableExtensions)
		{
			if (strcmp(required, available.extensionName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			HS_LOG(error, "Required extension not available: %s", required);
		}
	}

	return extensions;
}

bool RHIContextVulkanImpl::checkValidationLayerSupport()
{
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : s_validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

void RHIContextVulkanImpl::createDefaultPools()
{
	// Create default command pool
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = _device.queueFamilyIndices.graphics;

	VK_CHECK_RESULT(vkCreateCommandPool(_device, &poolInfo, nullptr, &_defaultCommandPool));

	// Create default descriptor pool
	std::array<VkDescriptorPoolSize, 6> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1000;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1000;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[2].descriptorCount = 1000;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[3].descriptorCount = 1000;
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[4].descriptorCount = 1000;
	poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	poolSizes[5].descriptorCount = 1000;

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = 1000;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VK_CHECK_RESULT(vkCreateDescriptorPool(_device, &descriptorPoolInfo, nullptr, &_defaultDescriptorPool));

	HS_LOG(info, "Created default command and descriptor pools");
}

void RHIContextVulkanImpl::Suspend(Swapchain* swapchain)
{
	// Suspend swapchain rendering
	if (swapchain)
	{
		SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
		// Implementation depends on SwapchainVulkan class
		vkDeviceWaitIdle(_device);
	}
}

void RHIContextVulkanImpl::Restore(Swapchain* swapchain)
{
	// Restore swapchain rendering
	if (swapchain)
	{
		SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
		// Recreate swapchain if needed
	}
}

uint32 RHIContextVulkanImpl::AcquireNextImage(Swapchain* swapchain)
{
	if (!swapchain) return UINT32_MAX;

	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	
	// This would typically be implemented in SwapchainVulkan class
	// For now, return 0 as placeholder
	return 0;
}

VkSurfaceKHR RHIContextVulkanImpl::createSurface(const NativeWindow& nativeWindow)
{
	VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)hs_platform_get_hinstance();
	surfaceCreateInfo.hwnd = static_cast<HWND>(nativeWindow.handle);
	
	VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &surface));
#elif defined(__APPLE__)
	// macOS surface creation would go here
#elif defined(__linux__)
	// Linux surface creation would go here
#endif

	return surface;
}

Swapchain* RHIContextVulkanImpl::CreateSwapchain(SwapchainInfo info)
{
	VkSurfaceKHR surface = createSurface(*info.nativeWindow);
	if (surface == VK_NULL_HANDLE)
	{
		HS_LOG(error, "Failed to create surface for swapchain");
		return nullptr;
	}

	SwapchainVulkan* swapchainVK = new SwapchainVulkan(info, this, _instance, _device, surface);
	
	HS_LOG(info, "Created Vulkan swapchain");
	return static_cast<Swapchain*>(swapchainVK);
}

void RHIContextVulkanImpl::DestroySwapchain(Swapchain* swapchain)
{
	if (!swapchain) return;

	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	delete swapchainVK;
	
	HS_LOG(info, "Destroyed Vulkan swapchain");
}

RenderPass* RHIContextVulkanImpl::CreateRenderPass(const RenderPassInfo& info)
{
	// Calculate total attachments
	uint32 attachmentCount = info.colorAttachmentCount;
	if (info.useDepthStencilAttachment)
		attachmentCount++;

	std::vector<VkAttachmentDescription> attachments(attachmentCount);
	std::vector<VkAttachmentReference> colorAttachmentRefs(info.colorAttachmentCount);
	
	// Color attachments
	for (uint32 i = 0; i < info.colorAttachmentCount; i++)
	{
		attachments[i].format = RHIUtilityVulkan::ToPixelFormat(info.colorAttachments[i].format);
		attachments[i].samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Support MSAA
		attachments[i].loadOp = RHIUtilityVulkan::ToLoadOp(info.colorAttachments[i].loadAction);
		attachments[i].storeOp = RHIUtilityVulkan::ToStoreOp(info.colorAttachments[i].storeAction);
		attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // TODO: Make configurable
		attachments[i].flags = 0;

		colorAttachmentRefs[i].attachment = i;
		colorAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	// Depth attachment
	VkAttachmentReference depthAttachmentRef{};
	if (info.useDepthStencilAttachment)
	{
		uint32 depthIndex = info.colorAttachmentCount;
		attachments[depthIndex].format = RHIUtilityVulkan::ToPixelFormat(info.depthStencilAttachment.format);
		attachments[depthIndex].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[depthIndex].loadOp = RHIUtilityVulkan::ToLoadOp(info.depthStencilAttachment.loadAction);
		attachments[depthIndex].storeOp = RHIUtilityVulkan::ToStoreOp(info.depthStencilAttachment.storeAction);
		attachments[depthIndex].stencilLoadOp = RHIUtilityVulkan::ToLoadOp(info.depthStencilAttachment.stencilLoadAction);
		attachments[depthIndex].stencilStoreOp = RHIUtilityVulkan::ToStoreOp(info.depthStencilAttachment.stencilStoreAction);
		attachments[depthIndex].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[depthIndex].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[depthIndex].flags = 0;

		depthAttachmentRef.attachment = depthIndex;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// Subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = info.colorAttachmentCount;
	subpass.pColorAttachments = colorAttachmentRefs.data();
	if (info.useDepthStencilAttachment)
	{
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	}

	// Subpass dependencies
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// Create render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkRenderPass vkRenderPass;
	VK_CHECK_RESULT(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &vkRenderPass));

	RenderPassVulkan* renderPass = new RenderPassVulkan(info);
	renderPass->handle = vkRenderPass;

	HS_LOG(info, "Created Vulkan render pass");
	return static_cast<RenderPass*>(renderPass);
}

void RHIContextVulkanImpl::DestroyRenderPass(RenderPass* renderPass)
{
	if (!renderPass) return;

	RenderPassVulkan* renderPassVK = static_cast<RenderPassVulkan*>(renderPass);
	if (renderPassVK->handle)
	{
		vkDestroyRenderPass(_device, renderPassVK->handle, nullptr);
	}
	delete renderPassVK;

	HS_LOG(info, "Destroyed Vulkan render pass");
}

Framebuffer* RHIContextVulkanImpl::CreateFramebuffer(const FramebufferInfo& info)
{
	RenderPassVulkan* renderPassVK = static_cast<RenderPassVulkan*>(info.renderPass);
	
	std::vector<VkImageView> attachments;
	attachments.reserve(info.colorAttachmentCount + (info.depthStencilAttachment ? 1 : 0));

	// Add color attachments
	for (uint32 i = 0; i < info.colorAttachmentCount; i++)
	{
		TextureVulkan* textureVK = static_cast<TextureVulkan*>(info.colorAttachments[i]);
		if (textureVK && textureVK->imageView)
		{
			attachments.push_back(textureVK->imageView);
		}
	}

	// Add depth attachment
	if (info.depthStencilAttachment)
	{
		TextureVulkan* depthTextureVK = static_cast<TextureVulkan*>(info.depthStencilAttachment);
		if (depthTextureVK && depthTextureVK->imageView)
		{
			attachments.push_back(depthTextureVK->imageView);
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

	VkFramebuffer vkFramebuffer;
	VK_CHECK_RESULT(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &vkFramebuffer));

	FramebufferVulkan* framebuffer = new FramebufferVulkan(info);
	framebuffer->handle = vkFramebuffer;

	HS_LOG(info, "Created Vulkan framebuffer (%dx%d)", info.width, info.height);
	return static_cast<Framebuffer*>(framebuffer);
}

void RHIContextVulkanImpl::DestroyFramebuffer(Framebuffer* framebuffer)
{
	if (!framebuffer) return;

	FramebufferVulkan* framebufferVK = static_cast<FramebufferVulkan*>(framebuffer);
	if (framebufferVK->handle)
	{
		vkDestroyFramebuffer(_device, framebufferVK->handle, nullptr);
	}
	delete framebufferVK;

	HS_LOG(info, "Destroyed Vulkan framebuffer");
}

GraphicsPipeline* RHIContextVulkanImpl::CreateGraphicsPipeline(const GraphicsPipelineInfo& info)
{
	// Shader stages
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	
	if (info.vertexShader)
	{
		ShaderVulkan* vertexShaderVK = static_cast<ShaderVulkan*>(info.vertexShader);
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertexShaderVK->handle;
		vertShaderStageInfo.pName = "main"; // TODO: Make configurable
		shaderStages.push_back(vertShaderStageInfo);
	}

	if (info.fragmentShader)
	{
		ShaderVulkan* fragmentShaderVK = static_cast<ShaderVulkan*>(info.fragmentShader);
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragmentShaderVK->handle;
		fragShaderStageInfo.pName = "main";
		shaderStages.push_back(fragShaderStageInfo);
	}

	// Vertex input
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	
	// TODO: Parse vertex layout from info.vertexLayout
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32>(bindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = RHIUtilityVulkan::ToPrimitiveTopology(info.primitiveTopology);
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport and scissor
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = 1920.0f; // TODO: Get from info
	viewport.height = 1080.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {1920, 1080};

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // TODO: From info
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = RHIUtilityVulkan::ToCullMode(info.rasterState.cullMode);
	rasterizer.frontFace = RHIUtilityVulkan::ToFrontFace(info.rasterState.frontFace);
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Depth stencil
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = info.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
	depthStencil.depthWriteEnable = info.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
	depthStencil.depthCompareOp = RHIUtilityVulkan::ToCompareOp(info.depthStencilState.depthCompareOp);
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = info.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;

	// Color blending
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(info.blendState.attachmentCount);
	for (uint32 i = 0; i < info.blendState.attachmentCount; i++)
	{
		auto& attachment = colorBlendAttachments[i];
		auto& srcAttachment = info.blendState.attachments[i];
		
		attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		attachment.blendEnable = srcAttachment.blendEnable ? VK_TRUE : VK_FALSE;
		attachment.srcColorBlendFactor = RHIUtilityVulkan::ToBlendFactor(srcAttachment.srcColorBlendFactor);
		attachment.dstColorBlendFactor = RHIUtilityVulkan::ToBlendFactor(srcAttachment.dstColorBlendFactor);
		attachment.colorBlendOp = RHIUtilityVulkan::ToBlendOp(srcAttachment.colorBlendOp);
		attachment.srcAlphaBlendFactor = RHIUtilityVulkan::ToBlendFactor(srcAttachment.srcAlphaBlendFactor);
		attachment.dstAlphaBlendFactor = RHIUtilityVulkan::ToBlendFactor(srcAttachment.dstAlphaBlendFactor);
		attachment.alphaBlendOp = RHIUtilityVulkan::ToBlendOp(srcAttachment.alphaBlendOp);
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = static_cast<uint32>(colorBlendAttachments.size());
	colorBlending.pAttachments = colorBlendAttachments.data();

	// Dynamic state
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Pipeline layout (TODO: Get from info.resourceLayout)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	VkPipelineLayout pipelineLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

	// Create pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = static_cast<RenderPassVulkan*>(info.renderPass)->handle;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline vkPipeline;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline));

	GraphicsPipelineVulkan* pipeline = new GraphicsPipelineVulkan(info);
	pipeline->handle = vkPipeline;
	pipeline->layout = pipelineLayout;

	HS_LOG(info, "Created Vulkan graphics pipeline");
	return static_cast<GraphicsPipeline*>(pipeline);
}

void RHIContextVulkanImpl::DestroyGraphicsPipeline(GraphicsPipeline* pipeline)
{
	if (!pipeline) return;

	GraphicsPipelineVulkan* pipelineVK = static_cast<GraphicsPipelineVulkan*>(pipeline);
	if (pipelineVK->layout)
	{
		vkDestroyPipelineLayout(_device, pipelineVK->layout, nullptr);
	}
	if (pipelineVK->handle)
	{
		vkDestroyPipeline(_device, pipelineVK->handle, nullptr);
	}
	delete pipelineVK;

	HS_LOG(info, "Destroyed Vulkan graphics pipeline");
}

Shader* RHIContextVulkanImpl::CreateShader(const ShaderInfo& info, const char* path)
{
	// Read shader file
	std::vector<char> code;
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		HS_LOG(error, "Failed to open shader file: %s", path);
		return nullptr;
	}

	size_t fileSize = (size_t)file.tellg();
	code.resize(fileSize);

	file.seekg(0);
	file.read(code.data(), fileSize);
	file.close();

	return CreateShader(info, code.data(), code.size());
}

Shader* RHIContextVulkanImpl::CreateShader(const ShaderInfo& info, const char* byteCode, size_t byteCodeSize)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = byteCodeSize;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode);

	VkShaderModule shaderModule;
	VK_CHECK_RESULT(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));

	ShaderVulkan* shader = new ShaderVulkan(info);
	shader->handle = shaderModule;

	HS_LOG(info, "Created Vulkan shader module");
	return static_cast<Shader*>(shader);
}

void RHIContextVulkanImpl::DestroyShader(Shader* shader)
{
	if (!shader) return;

	ShaderVulkan* shaderVK = static_cast<ShaderVulkan*>(shader);
	if (shaderVK->handle)
	{
		vkDestroyShaderModule(_device, shaderVK->handle, nullptr);
	}
	delete shaderVK;

	HS_LOG(info, "Destroyed Vulkan shader module");
}

Buffer* RHIContextVulkanImpl::CreateBuffer(const void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption)
{
	BufferInfo info;
	info.size = dataSize;
	info.usage = usage;
	info.memoryOption = memoryOption;
	
	return CreateBuffer(data, dataSize, info);
}

Buffer* RHIContextVulkanImpl::CreateBuffer(const void* data, size_t dataSize, const BufferInfo& info)
{
	// Create buffer
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = info.size;
	bufferInfo.usage = RHIUtilityVulkan::ToBufferUsage(info.usage);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer vkBuffer;
	VK_CHECK_RESULT(vkCreateBuffer(_device, &bufferInfo, nullptr, &vkBuffer));

	// Get memory requirements
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_device, vkBuffer, &memRequirements);

	// Determine memory properties
	VkMemoryPropertyFlags properties = 0;
	switch (info.memoryOption)
	{
	case EBufferMemoryOption::GPU_ONLY:
		properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;
	case EBufferMemoryOption::CPU_TO_GPU:
		properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		break;
	case EBufferMemoryOption::GPU_TO_CPU:
		properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
		break;
	}

	// Allocate memory
	VkDeviceMemory bufferMemory = VulkanMemoryAllocator::AllocateMemory(
		_device, _physicalDevice, memRequirements, properties);

	if (bufferMemory == VK_NULL_HANDLE)
	{
		vkDestroyBuffer(_device, vkBuffer, nullptr);
		HS_LOG(error, "Failed to allocate buffer memory");
		return nullptr;
	}

	// Bind memory
	vkBindBufferMemory(_device, vkBuffer, bufferMemory, 0);

	// Copy data if provided
	if (data && dataSize > 0 && (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
	{
		void* mappedData;
		vkMapMemory(_device, bufferMemory, 0, info.size, 0, &mappedData);
		memcpy(mappedData, data, dataSize);
		vkUnmapMemory(_device, bufferMemory);
	}
	else if (data && dataSize > 0)
	{
		// Need staging buffer for GPU-only memory
		// TODO: Implement staging buffer copy
	}

	BufferVulkan* buffer = new BufferVulkan(info);
	buffer->handle = vkBuffer;
	buffer->memory = bufferMemory;
	buffer->size = info.size;

	HS_LOG(info, "Created Vulkan buffer (size: %zu bytes)", info.size);
	return static_cast<Buffer*>(buffer);
}

void RHIContextVulkanImpl::DestroyBuffer(Buffer* buffer)
{
	if (!buffer) return;

	BufferVulkan* bufferVK = static_cast<BufferVulkan*>(buffer);
	if (bufferVK->handle)
	{
		vkDestroyBuffer(_device, bufferVK->handle, nullptr);
	}
	if (bufferVK->memory)
	{
		vkFreeMemory(_device, bufferVK->memory, nullptr);
	}
	delete bufferVK;

	HS_LOG(info, "Destroyed Vulkan buffer");
}

void RHIContextVulkanImpl::createImage(uint32 width, uint32 height, VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateImage(_device, &imageInfo, nullptr, &image));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(_device, image, &memRequirements);

	imageMemory = VulkanMemoryAllocator::AllocateMemory(
		_device, _physicalDevice, memRequirements, properties);

	vkBindImageMemory(_device, image, imageMemory, 0);
}

VkImageView RHIContextVulkanImpl::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VK_CHECK_RESULT(vkCreateImageView(_device, &viewInfo, nullptr, &imageView));

	return imageView;
}

Texture* RHIContextVulkanImpl::CreateTexture(void* imageData, const TextureInfo& info)
{
	return CreateTexture(imageData, info.width, info.height, info.format, info.type, info.usage);
}

Texture* RHIContextVulkanImpl::CreateTexture(void* imageData, uint32 width, uint32 height, 
	EPixelFormat format, ETextureType type, ETextureUsage usage)
{
	VkFormat vkFormat = RHIUtilityVulkan::ToPixelFormat(format);
	VkImageUsageFlags vkUsage = RHIUtilityVulkan::ToTextureUsage(usage);
	
	// Determine aspect flags
	VkImageAspectFlags aspectFlags = 0;
	if (usage & ETextureUsage::DEPTH_STENCIL_ATTACHMENT)
	{
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		// Add stencil aspect if format has stencil
		if (format == EPixelFormat::D24_UNORM_S8_UINT || format == EPixelFormat::D32_FLOAT_S8X24_UINT)
		{
			aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	// Create image
	VkImage image;
	VkDeviceMemory imageMemory;
	
	createImage(width, height, vkFormat, VK_IMAGE_TILING_OPTIMAL,
		vkUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

	// Create image view
	VkImageView imageView = createImageView(image, vkFormat, aspectFlags);

	// Copy data if provided
	if (imageData)
	{
		// Calculate image size
		// TODO: Proper size calculation based on format
		size_t imageSize = width * height * 4; // Assuming RGBA8
		
		// Create staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = imageSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		VK_CHECK_RESULT(vkCreateBuffer(_device, &bufferInfo, nullptr, &stagingBuffer));
		
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(_device, stagingBuffer, &memRequirements);
		
		stagingBufferMemory = VulkanMemoryAllocator::AllocateMemory(
			_device, _physicalDevice, memRequirements,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		vkBindBufferMemory(_device, stagingBuffer, stagingBufferMemory, 0);
		
		// Copy data to staging buffer
		void* data;
		vkMapMemory(_device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, imageData, imageSize);
		vkUnmapMemory(_device, stagingBufferMemory);
		
		// Transition image layout and copy
		transitionImageLayout(image, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer, image, width, height);
		transitionImageLayout(image, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		
		// Cleanup staging buffer
		vkDestroyBuffer(_device, stagingBuffer, nullptr);
		vkFreeMemory(_device, stagingBufferMemory, nullptr);
	}

	TextureInfo textureInfo;
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.format = format;
	textureInfo.type = type;
	textureInfo.usage = usage;

	TextureVulkan* texture = new TextureVulkan(textureInfo);
	texture->handle = image;
	texture->memory = imageMemory;
	texture->imageView = imageView;

	HS_LOG(info, "Created Vulkan texture (%dx%d)", width, height);
	return static_cast<Texture*>(texture);
}

void RHIContextVulkanImpl::DestroyTexture(Texture* texture)
{
	if (!texture) return;

	TextureVulkan* textureVK = static_cast<TextureVulkan*>(texture);
	if (textureVK->imageView)
	{
		vkDestroyImageView(_device, textureVK->imageView, nullptr);
	}
	if (textureVK->handle)
	{
		vkDestroyImage(_device, textureVK->handle, nullptr);
	}
	if (textureVK->memory)
	{
		vkFreeMemory(_device, textureVK->memory, nullptr);
	}
	delete textureVK;

	HS_LOG(info, "Destroyed Vulkan texture");
}

Sampler* RHIContextVulkanImpl::CreateSampler(const SamplerInfo& info)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR; // TODO: From info
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	
	// Get max anisotropy
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(_physicalDevice, &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkSampler vkSampler;
	VK_CHECK_RESULT(vkCreateSampler(_device, &samplerInfo, nullptr, &vkSampler));

	SamplerVulkan* sampler = new SamplerVulkan(info);
	sampler->handle = vkSampler;

	HS_LOG(info, "Created Vulkan sampler");
	return static_cast<Sampler*>(sampler);
}

void RHIContextVulkanImpl::DestroySampler(Sampler* sampler)
{
	if (!sampler) return;

	SamplerVulkan* samplerVK = static_cast<SamplerVulkan*>(sampler);
	if (samplerVK->handle)
	{
		vkDestroySampler(_device, samplerVK->handle, nullptr);
	}
	delete samplerVK;

	HS_LOG(info, "Destroyed Vulkan sampler");
}

ResourceLayout* RHIContextVulkanImpl::CreateResourceLayout()
{
	// TODO: Implement descriptor set layout creation
	ResourceLayoutVulkan* layout = new ResourceLayoutVulkan();
	
	HS_LOG(info, "Created Vulkan resource layout");
	return static_cast<ResourceLayout*>(layout);
}

void RHIContextVulkanImpl::DestroyResourceLayout(ResourceLayout* resourceLayout)
{
	if (!resourceLayout) return;

	ResourceLayoutVulkan* layoutVK = static_cast<ResourceLayoutVulkan*>(resourceLayout);
	// TODO: Destroy descriptor set layout
	delete layoutVK;

	HS_LOG(info, "Destroyed Vulkan resource layout");
}

ResourceSet* RHIContextVulkanImpl::CreateResourceSet()
{
	// TODO: Implement descriptor set allocation
	ResourceSetVulkan* resourceSet = new ResourceSetVulkan();
	
	HS_LOG(info, "Created Vulkan resource set");
	return static_cast<ResourceSet*>(resourceSet);
}

void RHIContextVulkanImpl::DestroyResourceSet(ResourceSet* resourceSet)
{
	if (!resourceSet) return;

	ResourceSetVulkan* resourceSetVK = static_cast<ResourceSetVulkan*>(resourceSet);
	// TODO: Free descriptor set
	delete resourceSetVK;

	HS_LOG(info, "Destroyed Vulkan resource set");
}

ResourceSetPool* RHIContextVulkanImpl::CreateResourceSetPool()
{
	// Create descriptor pool
	std::array<VkDescriptorPoolSize, 4> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 100;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 100;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[2].descriptorCount = 100;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[3].descriptorCount = 100;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 100;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	VkDescriptorPool descriptorPool;
	VK_CHECK_RESULT(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &descriptorPool));

	ResourceSetPoolVulkan* pool = new ResourceSetPoolVulkan();
	pool->handle = descriptorPool;

	HS_LOG(info, "Created Vulkan resource set pool");
	return static_cast<ResourceSetPool*>(pool);
}

void RHIContextVulkanImpl::DestroyResourceSetPool(ResourceSetPool* resourceSetPool)
{
	if (!resourceSetPool) return;

	ResourceSetPoolVulkan* poolVK = static_cast<ResourceSetPoolVulkan*>(resourceSetPool);
	if (poolVK->handle)
	{
		vkDestroyDescriptorPool(_device, poolVK->handle, nullptr);
	}
	delete poolVK;

	HS_LOG(info, "Destroyed Vulkan resource set pool");
}

CommandPool* RHIContextVulkanImpl::CreateCommandPool(uint32 queueFamilyIndex)
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndex;

	VkCommandPool vkCommandPool;
	VK_CHECK_RESULT(vkCreateCommandPool(_device, &poolInfo, nullptr, &vkCommandPool));

	CommandPoolVulkan* commandPool = new CommandPoolVulkan();
	commandPool->handle = vkCommandPool;

	HS_LOG(info, "Created Vulkan command pool");
	return static_cast<CommandPool*>(commandPool);
}

void RHIContextVulkanImpl::DestroyCommandPool(CommandPool* cmdPool)
{
	if (!cmdPool) return;

	CommandPoolVulkan* poolVK = static_cast<CommandPoolVulkan*>(cmdPool);
	if (poolVK->handle)
	{
		vkDestroyCommandPool(_device, poolVK->handle, nullptr);
	}
	delete poolVK;

	HS_LOG(info, "Destroyed Vulkan command pool");
}

CommandBuffer* RHIContextVulkanImpl::CreateCommandBuffer()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _defaultCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer vkCommandBuffer;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(_device, &allocInfo, &vkCommandBuffer));

	CommandBufferVulkan* commandBuffer = new CommandBufferVulkan();
	commandBuffer->handle = vkCommandBuffer;

	HS_LOG(info, "Created Vulkan command buffer");
	return static_cast<CommandBuffer*>(commandBuffer);
}

void RHIContextVulkanImpl::DestroyCommandBuffer(CommandBuffer* commandBuffer)
{
	if (!commandBuffer) return;

	CommandBufferVulkan* cmdBufferVK = static_cast<CommandBufferVulkan*>(commandBuffer);
	if (cmdBufferVK->handle)
	{
		vkFreeCommandBuffers(_device, _defaultCommandPool, 1, &cmdBufferVK->handle);
	}
	delete cmdBufferVK;

	HS_LOG(info, "Destroyed Vulkan command buffer");
}

Fence* RHIContextVulkanImpl::CreateFence()
{
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkFence vkFence;
	VK_CHECK_RESULT(vkCreateFence(_device, &fenceInfo, nullptr, &vkFence));

	FenceVulkan* fence = new FenceVulkan(_device);
	fence->handle = vkFence;

	HS_LOG(info, "Created Vulkan fence");
	return static_cast<Fence*>(fence);
}

void RHIContextVulkanImpl::DestroyFence(Fence* fence)
{
	if (!fence) return;

	FenceVulkan* fenceVK = static_cast<FenceVulkan*>(fence);
	if (fenceVK->handle)
	{
		vkDestroyFence(_device, fenceVK->handle, nullptr);
	}
	delete fenceVK;

	HS_LOG(info, "Destroyed Vulkan fence");
}

Semaphore* RHIContextVulkanImpl::CreateSemaphore()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkSemaphore vkSemaphore;
	VK_CHECK_RESULT(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &vkSemaphore));

	SemaphoreVulkan* semaphore = new SemaphoreVulkan(_device);
	semaphore->handle = vkSemaphore;

	HS_LOG(info, "Created Vulkan semaphore");
	return static_cast<Semaphore*>(semaphore);
}

void RHIContextVulkanImpl::DestroySemaphore(Semaphore* semaphore)
{
	if (!semaphore) return;

	SemaphoreVulkan* semaphoreVK = static_cast<SemaphoreVulkan*>(semaphore);
	if (semaphoreVK->handle)
	{
		vkDestroySemaphore(_device, semaphoreVK->handle, nullptr);
	}
	delete semaphoreVK;

	HS_LOG(info, "Destroyed Vulkan semaphore");
}

void RHIContextVulkanImpl::Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount)
{
	if (!swapchain || !buffers || bufferCount == 0) return;

	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);

	// Prepare command buffers
	std::vector<VkCommandBuffer> vkCommandBuffers(bufferCount);
	for (size_t i = 0; i < bufferCount; i++)
	{
		CommandBufferVulkan* cmdBufferVK = static_cast<CommandBufferVulkan*>(buffers[i]);
		vkCommandBuffers[i] = cmdBufferVK->handle;
	}

	// Submit info
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	// Wait semaphores
	VkSemaphore waitSemaphores[] = { /* swapchainVK->imageAvailableSemaphore */ };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 0; // TODO: Get from swapchain
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	
	// Command buffers
	submitInfo.commandBufferCount = static_cast<uint32>(vkCommandBuffers.size());
	submitInfo.pCommandBuffers = vkCommandBuffers.data();
	
	// Signal semaphores
	VkSemaphore signalSemaphores[] = { /* swapchainVK->renderFinishedSemaphore */ };
	submitInfo.signalSemaphoreCount = 0; // TODO: Get from swapchain
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK_RESULT(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

	HS_LOG(debug, "Submitted %zu command buffers", bufferCount);
}

void RHIContextVulkanImpl::Present(Swapchain* swapchain)
{
	if (!swapchain) return;

	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);
	
	// TODO: Implement presentation
	// This would typically call vkQueuePresentKHR
	
	HS_LOG(debug, "Presented swapchain");
}

void RHIContextVulkanImpl::WaitForIdle() const
{
	vkDeviceWaitIdle(_device);
	HS_LOG(debug, "Device idle");
}

VkCommandBuffer RHIContextVulkanImpl::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _defaultCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void RHIContextVulkanImpl::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_graphicsQueue);

	vkFreeCommandBuffers(_device, _defaultCommandPool, 1, &commandBuffer);
}

void RHIContextVulkanImpl::transitionImageLayout(VkImage image, VkFormat format, 
	VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

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
		endSingleTimeCommands(commandBuffer);
		return;
	}

	vkCmdPipelineBarrier(commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(commandBuffer);
}

void RHIContextVulkanImpl::copyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

VkResult RHIContextVulkanImpl::createDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void RHIContextVulkanImpl::destroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

void RHIContextVulkanImpl::setDebugObjectName(VkObjectType type, uint64 handle, const char* name)
{
	if (!s_enableValidationLayers) return;

	auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
		_instance, "vkSetDebugUtilsObjectNameEXT");
	if (!func) return;

	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = type;
	nameInfo.objectHandle = handle;
	nameInfo.pObjectName = name;

	func(_device, &nameInfo);
}

// Factory function to create the implementation
RHIContext* CreateRHIContextVulkan()
{
	return new RHIContextVulkanImpl();
}

HS_NS_END
