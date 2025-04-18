#include "Engine/RHI/Vulkan/RHIContextVulkan.h"

#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"
#include "Engine/RHI/Vulkan/RenderHandleVulkan.h"
#include "Engine/RHI/Vulkan/ResourceHandleVulkan.h"
#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"
#include "Engine/RHI/Vulkan/SwapchainVulkan.h"

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

static VKAPI_ATTR VkBool32 VKAPI_CALL hs_rhi_report_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
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
	}

	return VK_FALSE;
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
	// Create a Vulkan swapchain

	VkSurfaceKHR surface = createSurface(*reinterpret_cast<NativeWindowHandle*>(info.nativeWindowHandle));
	new SwapchainVulkan();
	return nullptr;
}

void RHIContextVulkan::DestroySwapchain(Swapchain* swapchain)
{
	// Destroy the Vulkan swapchain
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

CommandPool* RHIContextVulkan::CreateCommandPool()
{
	// Create a Vulkan command pool
	return nullptr;
}

void RHIContextVulkan::DestroyCommandPool(CommandPool* commandPool)
{
	// Destroy the Vulkan command pool
}

CommandBuffer* RHIContextVulkan::CreateCommandBuffer()
{
	// Create a Vulkan command buffer
	return nullptr;
}

void RHIContextVulkan::DestroyCommandBuffer(CommandBuffer* commandBuffer)
{
	// Destroy the Vulkan command buffer
}


Fence* RHIContextVulkan::CreateFence()
{
	// Create a Vulkan fence
	return nullptr;
}

void RHIContextVulkan::DestroyFence(Fence* fence)
{
	// Destroy the Vulkan fence
}

Semaphore* RHIContextVulkan::CreateSemaphore()
{
	// Create a Vulkan semaphore
	return nullptr;
}

void RHIContextVulkan::DestroySemaphore(Semaphore* semaphore)
{
	// Destroy the Vulkan semaphore
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

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = sdlExtensionCount;
	createInfo.ppEnabledExtensionNames = sdlExtensions;

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
		createInfo.enabledLayerCount = s_validationLayers.size();
		createInfo.ppEnabledLayerNames = s_validationLayers.data();
	}

	VK_CHECK_RESULT_AND_RETURN(vkCreateInstance(&createInfo, nullptr, &_instance));

	if (useValidationLayers)
	{
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.pNext = nullptr;
		debugCreateInfo.flags = 0;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = hs_rhi_report_debug_callback;
		debugCreateInfo.pUserData = nullptr;

		VK_CHECK_RESULT_AND_RETURN(createDebugUtilsMessengerEXT(_instance, &debugCreateInfo, &_debugMessenger, nullptr));
	}
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