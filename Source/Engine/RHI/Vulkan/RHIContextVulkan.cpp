#include "Engine/RHI/Vulkan/RHIContextVulkan.h"

#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"
#include "Engine/RHI/Vulkan/RenderHandleVulkan.h"
#include "Engine/RHI/Vulkan/ResourceHandleVulkan.h"
#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"
#include "Engine/RHI/Vulkan/SwapchainVulkan.h"

#include "Engine/Core/Window.h"

#include <SDL3/SDL_vulkan.h>


HS_NS_BEGIN

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
	if (messageType & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		HS_LOG(crash, "ERROR: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
	}
	else if (messageType & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		HS_LOG(warning, "WARNING: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
	}
	else if (messageType & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		HS_LOG(warning, "PERFORMANCE WARNING: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
	}
	else if (messageType & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		HS_LOG(info, "INFO: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
	}
	else if (messageType & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		HS_LOG(debug, "DEBUG: [%s] Code %i : %s", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage);
	}

	return VK_FALSE;
}

void RHIContextVulkan::setDebugObjectName(VkObjectType type, uint64 handle, const char* name)
{
	static auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(_instance, "vkSetDebugUtilsObjectNameEXT");
	HS_ASSERT(func, "function addr is nullptr");

	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = type;
	nameInfo.objectHandle = handle;
	nameInfo.pObjectName = name;
	nameInfo.pNext = nullptr;
	
	func(_device, &nameInfo);
}

VkResult RHIContextVulkan::createDebugUtilsMessengerEXT
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

void RHIContextVulkan::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* npAllocator = nullptr)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, pDebugMessenger, npAllocator);
	}
}

RHIContextVulkan::~RHIContextVulkan()
{
	Finalize();
}

bool RHIContextVulkan::Initialize()
{
	if (false == createInstance())
	{
		return false;
	}

	if (false == _device.Create(_instance))
	{
		return false;
	}

	return true;
}

void RHIContextVulkan::Finalize()
{
	// Cleanup Vulkan resources
}

uint32 RHIContextVulkan::AcquireNextImage(Swapchain* swapchain)
{
	// Acquire the next image from the swapchain
	return 0;
}

Swapchain* RHIContextVulkan::CreateSwapchain(SwapchainInfo info)
{
	VkSurfaceKHR surface = createSurface(*reinterpret_cast<NativeWindowHandle*>(info.nativeWindowHandle));
	SwapchainVulkan* swapchainVK = new SwapchainVulkan(info, this, _device, surface);

	return static_cast<Swapchain*>(swapchainVK);
}

void RHIContextVulkan::DestroySwapchain(Swapchain* swapchain)
{
	SwapchainVulkan* swapchainVK = static_cast<SwapchainVulkan*>(swapchain);

	delete swapchainVK;
}

RenderPass* RHIContextVulkan::CreateRenderPass(const RenderPassInfo& info)
{
	// Create a Vulkan render pass
	
	return nullptr;
}

void RHIContextVulkan::DestroyRenderPass(RenderPass* renderPass)
{
	// Destroy the Vulkan render pass
}

Framebuffer* RHIContextVulkan::CreateFramebuffer(const FramebufferInfo& info)
{
	// Create a Vulkan framebuffer
	return nullptr;
}

void RHIContextVulkan::DestroyFramebuffer(Framebuffer* framebuffer)
{
	// Destroy the Vulkan framebuffer
}

GraphicsPipeline* RHIContextVulkan::CreateGraphicsPipeline(const GraphicsPipelineInfo& info)
{
	// Create a Vulkan graphics pipeline
	return nullptr;
}

void RHIContextVulkan::DestroyGraphicsPipeline(GraphicsPipeline* pipeline)
{
	// Destroy the Vulkan graphics pipeline
}

Shader* RHIContextVulkan::CreateShader(EShaderStage stage, const char* path, const char* entryName, bool isBuiltIn)
{
	// Create a Vulkan shader
	return nullptr;
}

Shader* RHIContextVulkan::CreateShader(EShaderStage stage, const char* byteCode, size_t byteCodeSize, const char* entryName, bool isBuiltIn)
{
	// Create a Vulkan shader from byte code
	return nullptr;
}

void RHIContextVulkan::DestroyShader(Shader* shader)
{
	// Destroy the Vulkan shader
}

Buffer* RHIContextVulkan::CreateBuffer(void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption)
{
	// Create a Vulkan buffer
	return nullptr;
}

Buffer* RHIContextVulkan::CreateBuffer(void* data, size_t dataSize, const BufferInfo& info)
{
	// Create a Vulkan buffer with specific info
	return nullptr;
}

Texture* RHIContextVulkan::CreateTexture(void* image, const TextureInfo& info)
{
	// Create a Vulkan texture
	return nullptr;
}

Texture* RHIContextVulkan::CreateTexture(void* image, uint32 width, uint32 height, EPixelFormat format, ETextureType type, ETextureUsage usage)
{
	// Create a Vulkan texture with specific parameters
	return nullptr;
}

void RHIContextVulkan::DestroyTexture(Texture* texture)
{
	// Destroy the Vulkan texture
}

Sampler* RHIContextVulkan::CreateSampler(const SamplerInfo& info)
{
	// Create a Vulkan sampler
	return nullptr;
}

void RHIContextVulkan::DestroySampler(Sampler* sampler)
{
	// Destroy the Vulkan sampler
}

ResourceLayout* RHIContextVulkan::CreateResourceLayout()
{
	// Create a Vulkan resource layout
	return nullptr;
}

void RHIContextVulkan::DestroyResourceLayout(ResourceLayout* resourceLayout)
{
	// Destroy the Vulkan resource layout
}

ResourceSet* RHIContextVulkan::CreateResourceSet()
{
	// Create a Vulkan resource set
	return nullptr;
}

void RHIContextVulkan::DestroyResourceSet(ResourceSet* resourceSet)
{
	// Destroy the Vulkan resource set
}

CommandPool* RHIContextVulkan::CreateCommandPool(uint32 queueFamilyIndex)
{
	// Create a Vulkan command pool
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.queueFamilyIndex = queueFamilyIndex;

	VkCommandPool commandPool;
	return nullptr;
}

void RHIContextVulkan::DestroyCommandPool(CommandPool* commandPool)
{
	// Destroy the Vulkan command pool
}

CommandBuffer* RHIContextVulkan::CreateCommandBuffer()
{
	// Create a Vulkan command buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _defaultCommandPool;
	allocInfo.commandBufferCount = 1;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer vkCommandBuffer;
	vkAllocateCommandBuffers(_device, nullptr, &vkCommandBuffer);

	CommandBufferVulkan* commandBuffer = new CommandBufferVulkan();
	commandBuffer->commandBufferVK = vkCommandBuffer;

	return static_cast<CommandBuffer*>(commandBuffer);
}

void RHIContextVulkan::DestroyCommandBuffer(CommandBuffer* commandBuffer)
{
	// Destroy the Vulkan command buffer
	CommandBufferVulkan* commandBufferVK = static_cast<CommandBufferVulkan*>(commandBuffer);
	delete commandBuffer;
}


Fence* RHIContextVulkan::CreateFence()
{
	// Create a Vulkan fence
	FenceVulkan* fenceVK = new FenceVulkan(_device);
	return static_cast<Fence*>(fenceVK);
}

void RHIContextVulkan::DestroyFence(Fence* fence)
{
	// Destroy the Vulkan fence
	FenceVulkan* fenceVK = static_cast<FenceVulkan*>(fence);
	delete fence;
}

Semaphore* RHIContextVulkan::CreateSemaphore()
{
	// Create a Vulkan semaphore
	SemaphoreVulkan* semaphoreVK = new SemaphoreVulkan(_device);

	return static_cast<Semaphore*>(semaphoreVK);
}

void RHIContextVulkan::DestroySemaphore(Semaphore* semaphore)
{
	SemaphoreVulkan* semaphoreVK = static_cast<SemaphoreVulkan*>(semaphore);
	delete semaphoreVK;
}

void RHIContextVulkan::Submit(Swapchain* swapchain, CommandBuffer** buffers, size_t bufferCount)
{

}

void RHIContextVulkan::Present(Swapchain* swapchain)
{
	// Present the Vulkan swapchain
}

void RHIContextVulkan::WaitForIdle() const
{
	// Wait for the Vulkan device to be idle

}

bool RHIContextVulkan::createInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "HSMR";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
	appInfo.pEngineName = "HSMR";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	uint32 sdlExtensionCount = 0;
	const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = sdlExtensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = sdlExtensions;
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


	if (useValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.flags = 0;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = hs_rhi_vk_report_debug_callback;
		debugCreateInfo.pUserData = nullptr;
		debugCreateInfo.pNext = instanceCreateInfo.pNext;

		instanceCreateInfo.pNext = &debugCreateInfo;

		VK_CHECK_RESULT_AND_RETURN(createDebugUtilsMessengerEXT(_instance, &debugCreateInfo, &_debugMessenger, nullptr));
	}

	//uint32 extensionCount = 0;
	//vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	//if (extensionCount > 0)
	//{
	//	std::vector<VkExtensionProperties> extensions;
	//	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	//	for (VkExtensionProperties& extension : extensions)
	//	{
	//		_supportedInstanceExtensions.push_back(extension.extensionName);
	//	}
	//}

	VK_CHECK_RESULT_AND_RETURN(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));
}

VkSurfaceKHR RHIContextVulkan::createSurface(const NativeWindowHandle& windowHandle)
{
	SDL_Window* window = static_cast<SDL_Window*>(windowHandle.window);
	VkSurfaceKHR surface;
	SDL_Vulkan_CreateSurface(window, _instance, nullptr, &surface);

	return surface;
}

void RHIContextVulkan::cleanup()
{
	if (s_enableValidationLayers)
	{
		destroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}
	delete _device;

	vkDestroyInstance(_instance, nullptr);
}

HS_NS_END