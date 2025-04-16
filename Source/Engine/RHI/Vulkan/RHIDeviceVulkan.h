#ifndef __HS_RHI_DEVICE_VULKAN_H__
#define __HS_RHI_DEVICE_VULKAN_H__

#include "Precompile.h"

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

HS_NS_BEGIN


class RHIDeviceVulkan final
{
public:
	RHIDeviceVulkan(VkInstance instance);
	~RHIDeviceVulkan();
	
	void Destroy();
	
	struct QueueFamilyIndices
	{
		uint32 graphics;
		uint32 compute;
		uint32 blit;
	} queueFamilyIndices;

	VkDevice logicalDevice;
	VkPhysicalDevice physicalDevice;

	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	std::vector<const char*> supportedExtensions;
	std::vector<const char*> deviceExtensions;

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	VkCommandPool commandPool = VK_NULL_HANDLE;

	HS_FORCEINLINE operator VkDevice() { return logicalDevice;  }

private:
	void getPhysicalDevice();
	void createLogicalDevice();
	uint32 getPhysicalDeviceScore(VkPhysicalDevice device);

	VkInstance _instance;
};

HS_NS_END



#endif