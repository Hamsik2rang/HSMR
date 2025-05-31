#include "Engine/RHI/Vulkan/RHIContextVulkan.h"

#include "Engine/RHI/Vulkan/CommandHandleVulkan.h"
#include "Engine/RHI/Vulkan/RenderHandleVulkan.h"
#include "Engine/RHI/Vulkan/ResourceHandleVulkan.h"
#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"
#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"
#include "Engine/RHI/Vulkan/SwapchainVulkan.h"

#include "Engine/Core/Window.h"

#include "Engine/Platform/Windows/PlatformApplicationWindows.h"
#include "Engine/Core/FileSystem.h"


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

HS_NS_BEGIN

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

	createDefaultCommandPool();

	return true;
}

void RHIContextVulkan::Finalize()
{
	// Cleanup Vulkan resources
}

void RHIContextVulkan::Suspend(Swapchain* swapchain)
{

}

void RHIContextVulkan::Restore(Swapchain* swapchain)
{

}

uint32 RHIContextVulkan::AcquireNextImage(Swapchain* swapchain)
{
	// Acquire the next image from the swapchain
	return 0;
}

Swapchain* RHIContextVulkan::CreateSwapchain(SwapchainInfo info)
{
	VkSurfaceKHR surface = createSurface(*info.nativeWindow);
	SwapchainVulkan* swapchainVK = new SwapchainVulkan(info, this, _instance, _device, surface);

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
	GraphicsPipelineVulkan* pipelineVK = new GraphicsPipelineVulkan(info);

	VkPipeline pipelinevk;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	
	//vkCreateGraphicsPipelines()


	return static_cast<GraphicsPipeline*>(pipelineVK);
}

void RHIContextVulkan::DestroyGraphicsPipeline(GraphicsPipeline* pipeline)
{
	// Destroy the Vulkan graphics pipeline
}

Shader* RHIContextVulkan::CreateShader(const ShaderInfo& info, const char* path)
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
	size_t byteCodeSize = FileSystem::Read(fileHandle, byteCode, sizeof(byteCode));

	FileSystem::Close(fileHandle);

	// Create a Vulkan shader
	ShaderVulkan* shaderVK = static_cast<ShaderVulkan*>(CreateShader(info, byteCode, byteCodeSize));

	delete[] byteCode;

	return shaderVK;
}

Shader* RHIContextVulkan::CreateShader(const ShaderInfo& info, const char* byteCode, size_t byteCodeSize)
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

	ShaderVulkan* shaderVK = new ShaderVulkan(info);
	shaderVK->handle = shaderModule;

	return static_cast<Shader*>(shaderVK);
}

void RHIContextVulkan::DestroyShader(Shader* shader)
{
	ShaderVulkan* shaderVulkan = static_cast<ShaderVulkan*>(shader);
	if (shaderVulkan->handle)
	{
		vkDestroyShaderModule(_device, shaderVulkan->handle, nullptr);
		shaderVulkan->handle = VK_NULL_HANDLE;
	}
}

Buffer* RHIContextVulkan::CreateBuffer(void* data, size_t dataSize, EBufferUsage usage, EBufferMemoryOption memoryOption)
{
	VkBuffer bufferVk;
	// Create a Vulkan buffer
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = dataSize;
	createInfo.usage = RHIUtilityVulkan::ToBufferUsage(usage);
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.pQueueFamilyIndices = &_device.queueFamilyIndices.graphics;

	VK_CHECK_RESULT(vkCreateBuffer(_device, &createInfo, nullptr, &bufferVk));

	VkMemoryRequirements memReq{};
	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkGetBufferMemoryRequirements(_device, bufferVk, &memReq);
	memAllocInfo.allocationSize = memReq.size;
	
	return nullptr;
}

Buffer* RHIContextVulkan::CreateBuffer(void* data, size_t dataSize, const BufferInfo& info)
{
	// Create a Vulkan buffer with specific info
	return nullptr;
}

void RHIContextVulkan::DestroyBuffer(Buffer* buffer)
{

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

ResourceSetPool* RHIContextVulkan::CreateResourceSetPool()
{
	return nullptr;
}
void RHIContextVulkan::DestroyResourceSetPool(ResourceSetPool* resourceSetPool)
{

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
	vkAllocateCommandBuffers(_device, &allocInfo, &vkCommandBuffer);

	CommandBufferVulkan* commandBuffer = new CommandBufferVulkan();
	commandBuffer->handle = vkCommandBuffer;

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

// TODO: 이거 별로 안예쁨.
#ifdef CreateSemaphore
#define CreateSemaphoreTemp CreateSemaphore
#undef CreateSemaphore
#endif
Semaphore* RHIContextVulkan::CreateSemaphore()
{
	// Create a Vulkan semaphore
	SemaphoreVulkan* semaphoreVK = new SemaphoreVulkan(_device);

	return static_cast<Semaphore*>(semaphoreVK);
}
#ifdef CreateSemaphoreTemp CreateSemaphore
#define CreateSemaphore CreateSemaphoreTemp
#undef CreateSemaphoreTemp
#endif

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

void RHIContextVulkan::cleanup()
{
	if (s_enableValidationLayers)
	{
		destroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}
	delete _device;

	vkDestroyInstance(_instance, nullptr);
}
#pragma region >>> Implementation create functions

bool RHIContextVulkan::createInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "HSMR";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
	appInfo.pEngineName = "HSMR";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

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
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = hs_rhi_vk_report_debug_callback;
		debugCreateInfo.pUserData = nullptr;
		debugCreateInfo.pNext = instanceCreateInfo.pNext;

		instanceCreateInfo.pNext = &debugCreateInfo;
	}

	VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));

	VK_CHECK_RESULT(createDebugUtilsMessengerEXT(_instance, &debugCreateInfo, &_debugMessenger, nullptr));

	return true;
}

void RHIContextVulkan::createDefaultCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Command buffers can be reset individually
	poolInfo.queueFamilyIndex = _device.queueFamilyIndices.graphics;
	VK_CHECK_RESULT(vkCreateCommandPool(_device, &poolInfo, nullptr, &_defaultCommandPool));
}

VkSurfaceKHR RHIContextVulkan::createSurface(const NativeWindow& nativeWindow)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)hs_platform_get_hinstance();
	surfaceCreateInfo.hwnd = static_cast<HWND>(nativeWindow.handle);
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(_instance, &surfaceCreateInfo, nullptr, &surface));

	return surface;
}

VkRenderPass RHIContextVulkan::createRenderPass(const RenderPassInfo& info)
{
	auto attachmentCount = info.colorAttachmentCount + static_cast<uint8>(info.useDepthStencilAttachment);

	std::vector<VkAttachmentDescription> attachments(attachmentCount);
	int index = 0;
	for (; index < info.colorAttachmentCount; index++)
	{
		attachments[index].flags = 0;
		attachments[index].format = RHIUtilityVulkan::ToPixelFormat(info.colorAttachments[index].format);
		//...
	}

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = attachmentCount;
	createInfo.pNext = nullptr;
	createInfo.subpassCount = 1;

	return VK_NULL_HANDLE; // Placeholder, implement the rest of the function
}

VkFramebuffer RHIContextVulkan::createFramebuffer(const FramebufferInfo& info)
{

}

VkPipeline RHIContextVulkan::createGraphicsPipeline(const GraphicsPipelineInfo& info)
{

}

VkPipeline RHIContextVulkan::createComputePipeline(const ComputePipelineInfo& info)
{

}
#pragma endregion 

HS_NS_END