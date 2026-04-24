#ifndef __HS_RHI_DEVICE_VULKAN_H__
#define __HS_RHI_DEVICE_VULKAN_H__

#include "Precompile.h"

#include "RHI/Vulkan/VulkanUtility.h"

#include <string>
#include <vector>

HS_NS_BEGIN

class HS_RHI_API VulkanDevice final
{
public:
    static constexpr uint32 InvalidQueueFamily = HS_UINT32_MAX;

    VulkanDevice() = default;
    ~VulkanDevice();

    struct OptionalDeviceExtensions
    {
        union
        {
            uint64 mask = 0;
            struct
            {
                uint64 swapchain : 1;
                uint64 dynamicRendering : 1;
                uint64 descriptorIndexing : 1;
                uint64 getPhysicalDeviceProperties2 : 1;
                uint64 maintenance3 : 1;
                uint64 synchronization2 : 1;
            };
        };
    };

    struct OptionalDeviceFeatures
    {
        union
        {
            uint64 core10 = 0;
            struct
            {
                uint64 samplerAnisotropy : 1;
                uint64 fillModeNonSolid : 1;
            };
        };

        union
        {
            uint64 core12 = 0;
            struct
            {
                uint64 descriptorIndexing : 1;
                uint64 descriptorBindingVariableDescriptorCount : 1;
                uint64 runtimeDescriptorArray : 1;
                uint64 descriptorBindingPartiallyBound : 1;
                uint64 descriptorBindingSampledImageUpdateAfterBind : 1;
                uint64 shaderSampledImageArrayNonUniformIndexing : 1;
                uint64 bufferDeviceAddress : 1;
            };
        };

        union
        {
            uint64 core13 = 0;
            struct
            {
                uint64 dynamicRendering : 1;
                uint64 synchronization2 : 1;
            };
        };
    };
	
    bool Create(VkInstance instance);
    void Destroy();

    struct QueueFamilyIndices
    {
        uint32 graphics = InvalidQueueFamily;
        uint32 compute = InvalidQueueFamily;
        uint32 transfer = InvalidQueueFamily;
    } queueFamilyIndices;

    VkDevice logicalDevice          = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceVulkan11Features features11{};
    VkPhysicalDeviceVulkan12Features features12{};
    VkPhysicalDeviceVulkan13Features features13{};

    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkSurfaceKHR surface;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue  = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;

	std::vector<const char*> supportedExtensions;
	std::vector<const char*> deviceExtensions;
    std::vector<std::string> availableDeviceExtensions;

    OptionalDeviceExtensions optionalExtensions;
    OptionalDeviceFeatures optionalFeatures;

    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    VkCommandPool commandPool = VK_NULL_HANDLE;

	HS_FORCEINLINE operator VkDevice() { return logicalDevice;  }
    HS_FORCEINLINE const RHICapabilities& GetCapabilities() const { return _capabilities; }

private:
	void getPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	uint32 getPhysicalDeviceScore(VkPhysicalDevice device);
    void queryDeviceCapabilities(VkPhysicalDevice device);
    bool hasDeviceExtension(const char* extensionName) const;

    VkInstance _instanceVk;
    RHICapabilities _capabilities;
};

HS_NS_END



#endif
