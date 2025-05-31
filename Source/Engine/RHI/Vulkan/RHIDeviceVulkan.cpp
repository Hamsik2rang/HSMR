#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"

#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"

#include "Engine/Core/Log.h"
#include <algorithm>
#include <utility>

HS_NS_BEGIN

std::vector<const char*> s_requiredDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

RHIDeviceVulkan::~RHIDeviceVulkan()
{
	Destroy();
}

bool RHIDeviceVulkan::Create(VkInstance instance)
{
	_instance = instance;

	getPhysicalDevice();
	createLogicalDevice();
	return true;
}

void RHIDeviceVulkan::Destroy()
{
	vkDestroyDevice(logicalDevice, nullptr);
}

void RHIDeviceVulkan::getPhysicalDevice()
{
	uint32 physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, nullptr);
	vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices.data());

	uint32 maxScore = 0;
	for (uint32 i = 0; i < physicalDeviceCount; i++)
	{
		uint32 score = getPhysicalDeviceScore(physicalDevices[i]);
		bool isExtensionSupported = false;
		uint32 availableExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &availableExtensionCount, availableExtensions.data());

		int supportedExtensionCount = 0;
		for (const auto& extension : availableExtensions)
		{
			for (int j = 0; j < s_requiredDeviceExtensions.size(); j++)
			{
				if (strcmp(extension.extensionName, s_requiredDeviceExtensions[j]) == 0)
				{
					supportedExtensionCount++;
					break;
				}
			}
		}
		
		if ((supportedExtensionCount == s_requiredDeviceExtensions.size()) && (maxScore == 0 || maxScore < score))
		{
			maxScore = score;
			physicalDevice = physicalDevices[i];
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		HS_THROW("you don't have a physical device.");
	}

	vkGetPhysicalDeviceFeatures(physicalDevice, &features);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	HS_LOG(info, "Driver Version: %u", properties.driverVersion);
	HS_LOG(info, "Device Name:    %s", properties.deviceName);
	HS_LOG(info, "Device Type:    %d", properties.deviceType);
	HS_LOG(info, "API Version:    %d.%d.%d",
		(properties.apiVersion >> 22) & 0x3FF,
		(properties.apiVersion >> 12) & 0x3FF,
		(properties.apiVersion & 0xFFF));
}

void RHIDeviceVulkan::createLogicalDevice()
{
	// Set queue family indices
	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	bool isQueueFamilyFound = false;
	for (size_t i = 0; i < queueFamilyProperties.size(); i++)
	{
		const auto& queueFamily = queueFamilyProperties[i];
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices.graphics = i;
			queueFamilyIndices.compute = i;
		}
		else if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			queueFamilyIndices.compute = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			queueFamilyIndices.transfer = i;
		}
	}

	VkDeviceQueueCreateInfo queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
	queueInfo.queueCount = 1;
	float queuePriority = 0.0f;
	queueInfo.pQueuePriorities = &queuePriority;

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.pEnabledFeatures = &features;
	deviceInfo.enabledExtensionCount = s_requiredDeviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = s_requiredDeviceExtensions.data();
	
	VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicalDevice));

	vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphics, 0, &graphicsQueue);
	if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
	{
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices.compute, 0, &computeQueue);
	}
	else
	{
		computeQueue = graphicsQueue;
	}

	if (queueFamilyIndices.transfer != queueFamilyIndices.graphics)
	{
		vkGetDeviceQueue(logicalDevice, queueFamilyIndices.transfer, 0, &transferQueue);
	}
	else
	{
		transferQueue = graphicsQueue;
	}
}

uint32 RHIDeviceVulkan::getPhysicalDeviceScore(VkPhysicalDevice physicalDevice)
{

	VkPhysicalDeviceProperties properties{};
	VkPhysicalDeviceFeatures features{};

	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);

	uint32 score = 0;

	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
	{
		score += 200;
	}

	score += properties.limits.maxColorAttachments;
	score += properties.limits.framebufferColorSampleCounts;
	//score += properties.limits.maxImageDimension2D;
	//score += properties.limits.maxUniformBufferRange;
	//...

	return score;
}

void RHIDeviceVulkan::createSurface()
{

}


HS_NS_END