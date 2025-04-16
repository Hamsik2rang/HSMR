#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"

#include "Engine/RHI/Vulkan/RHIUtilityVulkan.h"

#include "Engine/Core/Log.h"

HS_NS_BEGIN

RHIDeviceVulkan::RHIDeviceVulkan(VkInstance instance)
	: physicalDevice{ VK_NULL_HANDLE }
	, logicalDevice{ VK_NULL_HANDLE }
	, _instance(instance)
{
	getPhysicalDevice();
	createLogicalDevice();
}

RHIDeviceVulkan::~RHIDeviceVulkan()
{
	Destroy();
}

void RHIDeviceVulkan::Destroy()
{
	vkDestroyDevice(logicalDevice, nullptr);
}

void RHIDeviceVulkan::getPhysicalDevice()
{
	uint32 physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);
	VkPhysicalDevice* physicalDevices = new VkPhysicalDevice[physicalDeviceCount];

	uint32 maxScore = 0;
	for (uint32 i = 0; i < physicalDeviceCount; i++)
	{
		uint32 score = getPhysicalDeviceScore(physicalDevices[i]);
		if (maxScore == 0 || maxScore < score)
		{
			uint32 queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilyProperties.data());

			bool isQueueFamilyFound = false;
			for (const auto& queueFamily : queueFamilyProperties)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					queueFamilyIndices.graphics = i;
					isQueueFamilyFound = true;
				}
			}

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
	VkDeviceQueueCreateInfo queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
	queueInfo.queueCount = 1;
	float queuePriority = 0.0f;
	queueInfo.pQueuePriorities = &queuePriority;

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.pEnabledFeatures = &features;
	deviceInfo.enabledExtensionCount = 0;

	VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicalDevice));
}

uint32 RHIDeviceVulkan::getPhysicalDeviceScore(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;

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

	score += properties.limits.maxImageDimension2D;
	score += properties.limits.maxColorAttachments;
	score += properties.limits.framebufferColorSampleCounts;
	score += properties.limits.maxUniformBufferRange;
	//...

	return score;
}



HS_NS_END