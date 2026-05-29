#include "RHI/Vulkan/VulkanDevice.h"

#include "RHI/Vulkan/VulkanUtility.h"

#include "Core/Log.h"
#include <algorithm>
#include <array>
#include <utility>

HS_NS_BEGIN

static constexpr std::array<const char*, 1> s_requiredDeviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanDevice::~VulkanDevice()
{
    Destroy();
}

bool VulkanDevice::Create(VkInstance instance)
{
    _instanceVk = instance;

    getPhysicalDevice();
    createLogicalDevice();
    return true;
}

void VulkanDevice::Destroy()
{
    if (logicalDevice == VK_NULL_HANDLE)
    {
        return;
    }

    vkDestroyDevice(logicalDevice, nullptr);
    logicalDevice = VK_NULL_HANDLE;
    graphicsQueue = VK_NULL_HANDLE;
    computeQueue = VK_NULL_HANDLE;
    transferQueue = VK_NULL_HANDLE;
}

void VulkanDevice::getPhysicalDevice()
{
    uint32 physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(_instanceVk, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, nullptr);
    vkEnumeratePhysicalDevices(_instanceVk, &physicalDeviceCount, physicalDevices.data());

    uint32 maxScore = 0;
    for (uint32 i = 0; i < physicalDeviceCount; i++)
    {
        uint32 score                   = getPhysicalDeviceScore(physicalDevices[i]);
        uint32 availableExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &availableExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevices[i], nullptr, &availableExtensionCount, availableExtensions.data());

        int supportedExtensionCount = 0;
        for (const auto& extension : availableExtensions)
        {
            for (uint32 j = 0; j < static_cast<uint32>(s_requiredDeviceExtensions.size()); j++)
            {
                if (strcmp(extension.extensionName, s_requiredDeviceExtensions[j]) == 0)
                {
                    supportedExtensionCount++;
                    break;
                }
            }
        }

        if ((supportedExtensionCount == static_cast<int>(s_requiredDeviceExtensions.size())) && (maxScore == 0 || maxScore < score))
        {
            maxScore       = score;
            physicalDevice = physicalDevices[i];
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        HS_THROW("you don't have a physical device.");
    }
    queryDeviceCapabilities(physicalDevice);
    HS_LOG(info, "Driver Version: %u", properties.driverVersion);
    HS_LOG(info, "Device Name:    %s", properties.deviceName);
    HS_LOG(info, "Device Type:    %d", properties.deviceType);
    HS_LOG(info, "API Version:    %d.%d.%d", (properties.apiVersion >> 22) & 0x3FF, (properties.apiVersion >> 12) & 0x3FF, (properties.apiVersion & 0xFFF));
}

void VulkanDevice::createLogicalDevice()
{
    // Set queue family indices
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    queueFamilyIndices = QueueFamilyIndices{};
    for (uint32 i = 0; i < static_cast<uint32>(queueFamilyProperties.size()); i++)
    {
        const auto& queueFamily = queueFamilyProperties[i];
        const bool supportsGraphics = (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
        const bool supportsCompute = (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
        const bool supportsTransfer = (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0;

        if (supportsGraphics && queueFamilyIndices.graphics == InvalidQueueFamily)
        {
            queueFamilyIndices.graphics = i;
        }

        if (supportsCompute &&
            (queueFamilyIndices.compute == InvalidQueueFamily || !supportsGraphics))
        {
            queueFamilyIndices.compute = i;
        }

        if (supportsTransfer)
        {
            const bool isDedicatedTransfer = !supportsGraphics && !supportsCompute;
            if (queueFamilyIndices.transfer == InvalidQueueFamily || isDedicatedTransfer)
            {
                queueFamilyIndices.transfer = i;
            }
        }
    }

    if (queueFamilyIndices.graphics == InvalidQueueFamily)
    {
        HS_THROW("Vulkan device does not have a graphics queue family.");
    }

    if (queueFamilyIndices.compute == InvalidQueueFamily)
    {
        queueFamilyIndices.compute = queueFamilyIndices.graphics;
    }

    if (queueFamilyIndices.transfer == InvalidQueueFamily)
    {
        queueFamilyIndices.transfer = queueFamilyIndices.graphics;
    }

    std::vector<uint32> uniqueQueueFamilyIndices;
    auto appendUniqueQueueFamily = [&uniqueQueueFamilyIndices](uint32 queueFamilyIndex)
    {
        if (std::find(uniqueQueueFamilyIndices.begin(), uniqueQueueFamilyIndices.end(), queueFamilyIndex) ==
            uniqueQueueFamilyIndices.end())
        {
            uniqueQueueFamilyIndices.push_back(queueFamilyIndex);
        }
    };
    appendUniqueQueueFamily(queueFamilyIndices.graphics);
    appendUniqueQueueFamily(queueFamilyIndices.compute);
    appendUniqueQueueFamily(queueFamilyIndices.transfer);

    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    queueInfos.reserve(uniqueQueueFamilyIndices.size());
    for (uint32 queueFamilyIndex : uniqueQueueFamilyIndices)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueInfos.push_back(queueInfo);
    }

    std::vector<const char*> enabledExtensions(
        std::begin(s_requiredDeviceExtensions),
        std::end(s_requiredDeviceExtensions));
    uint32 apiMajor = (properties.apiVersion >> 22) & 0x3FF;
    uint32 apiMinor = (properties.apiVersion >> 12) & 0x3FF;
    if (_capabilities.renderingPath == ERHIRenderingPath::DynamicRendering &&
        !(apiMajor > 1 || apiMinor >= 3) &&
        optionalExtensions.dynamicRendering)
    {
        enabledExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    }
    if (optionalExtensions.descriptorIndexing)
    {
        enabledExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeatures.dynamicRendering = _capabilities.renderingPath == ERHIRenderingPath::DynamicRendering ? VK_TRUE : VK_FALSE;

    features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features11.shaderDrawParameters = VK_TRUE;
    features11.pNext = nullptr;

    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.descriptorIndexing = optionalFeatures.descriptorIndexing ? VK_TRUE : VK_FALSE;
    features12.descriptorBindingVariableDescriptorCount = optionalFeatures.descriptorBindingVariableDescriptorCount ? VK_TRUE : VK_FALSE;
    features12.runtimeDescriptorArray = optionalFeatures.runtimeDescriptorArray ? VK_TRUE : VK_FALSE;
    features12.descriptorBindingPartiallyBound = optionalFeatures.descriptorBindingPartiallyBound ? VK_TRUE : VK_FALSE;
    features12.descriptorBindingSampledImageUpdateAfterBind = optionalFeatures.descriptorBindingSampledImageUpdateAfterBind ? VK_TRUE : VK_FALSE;
    features12.shaderSampledImageArrayNonUniformIndexing = optionalFeatures.shaderSampledImageArrayNonUniformIndexing ? VK_TRUE : VK_FALSE;
    features12.bufferDeviceAddress = optionalFeatures.bufferDeviceAddress ? VK_TRUE : VK_FALSE;
    features12.pNext = &features11;

    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.synchronization2 = optionalFeatures.synchronization2 ? VK_TRUE : VK_FALSE;
    features13.dynamicRendering = optionalFeatures.dynamicRendering ? VK_TRUE : VK_FALSE;
    features13.pNext = &features12;

    void* featureChain = nullptr;
    if (apiMajor > 1 || apiMinor >= 3)
    {
        featureChain = &features13;
    }
    else if (apiMajor > 1 || apiMinor >= 2)
    {
        features12.pNext = &features11;
        featureChain = &features12;
    }
    else if (optionalExtensions.dynamicRendering && _capabilities.renderingPath == ERHIRenderingPath::DynamicRendering)
    {
        dynamicRenderingFeatures.pNext = &features11;
        featureChain = &dynamicRenderingFeatures;
    }
    else
    {
        featureChain = &features11;
    }
    

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount    = static_cast<uint32>(queueInfos.size());
    deviceInfo.pQueueCreateInfos       = queueInfos.data();
    deviceInfo.pEnabledFeatures        = &features;
    deviceInfo.enabledExtensionCount   = static_cast<uint32>(enabledExtensions.size());
    deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
    deviceInfo.pNext                   = featureChain;

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

uint32 VulkanDevice::getPhysicalDeviceScore(VkPhysicalDevice physicalDevice)
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
    // score += properties.limits.maxImageDimension2D;
    // score += properties.limits.maxUniformBufferRange;
    //...

    return score;
}

void VulkanDevice::createSurface()
{
}

void VulkanDevice::queryDeviceCapabilities(VkPhysicalDevice device)
{
    uint32 availableExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());

    availableDeviceExtensions.clear();
    optionalExtensions.mask = 0;
    for (const auto& extension : availableExtensions)
    {
        availableDeviceExtensions.push_back(extension.extensionName);
        if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            optionalExtensions.swapchain = 1;
        }
        else if (strcmp(extension.extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
        {
            optionalExtensions.dynamicRendering = 1;
        }
        else if (strcmp(extension.extensionName, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0)
        {
            optionalExtensions.descriptorIndexing = 1;
        }
        else if (strcmp(extension.extensionName, VK_KHR_MAINTENANCE3_EXTENSION_NAME) == 0)
        {
            optionalExtensions.maintenance3 = 1;
        }
        else if (strcmp(extension.extensionName, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0)
        {
            optionalExtensions.synchronization2 = 1;
        }
    }

    vkGetPhysicalDeviceFeatures(device, &features);
    vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
    vkGetPhysicalDeviceProperties(device, &properties);

    optionalFeatures.core10 = 0;
    optionalFeatures.core12 = 0;
    optionalFeatures.core13 = 0;
    optionalFeatures.samplerAnisotropy = features.samplerAnisotropy == VK_TRUE;
    optionalFeatures.fillModeNonSolid = features.fillModeNonSolid == VK_TRUE;

    uint32 apiMajor = (properties.apiVersion >> 22) & 0x3FF;
    uint32 apiMinor = (properties.apiVersion >> 12) & 0x3FF;

    if (apiMajor > 1 || apiMinor >= 2)
    {
        VkPhysicalDeviceVulkan12Features queryFeatures12{};
        VkPhysicalDeviceVulkan13Features queryFeatures13{};
        queryFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        queryFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        queryFeatures13.pNext = &queryFeatures12;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = (apiMajor > 1 || apiMinor >= 3) ? static_cast<void*>(&queryFeatures13) : static_cast<void*>(&queryFeatures12);
        vkGetPhysicalDeviceFeatures2(device, &features2);

        optionalFeatures.descriptorIndexing = queryFeatures12.descriptorIndexing == VK_TRUE;
        optionalFeatures.descriptorBindingVariableDescriptorCount = queryFeatures12.descriptorBindingVariableDescriptorCount == VK_TRUE;
        optionalFeatures.runtimeDescriptorArray = queryFeatures12.runtimeDescriptorArray == VK_TRUE;
        optionalFeatures.descriptorBindingPartiallyBound = queryFeatures12.descriptorBindingPartiallyBound == VK_TRUE;
        optionalFeatures.descriptorBindingSampledImageUpdateAfterBind = queryFeatures12.descriptorBindingSampledImageUpdateAfterBind == VK_TRUE;
        optionalFeatures.shaderSampledImageArrayNonUniformIndexing = queryFeatures12.shaderSampledImageArrayNonUniformIndexing == VK_TRUE;
        optionalFeatures.bufferDeviceAddress = queryFeatures12.bufferDeviceAddress == VK_TRUE;

        optionalFeatures.dynamicRendering = queryFeatures13.dynamicRendering == VK_TRUE;
        optionalFeatures.synchronization2 = queryFeatures13.synchronization2 == VK_TRUE;
    }
    else if (optionalExtensions.dynamicRendering)
    {
        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &dynamicRenderingFeatures;

        auto getPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
            vkGetInstanceProcAddr(_instanceVk, "vkGetPhysicalDeviceFeatures2KHR"));
        if (getPhysicalDeviceFeatures2KHR != nullptr)
        {
            getPhysicalDeviceFeatures2KHR(device, &features2);
            optionalFeatures.dynamicRendering = (dynamicRenderingFeatures.dynamicRendering == VK_TRUE);
        }
    }

    bool supportsBindless =
        optionalFeatures.descriptorIndexing &&
        optionalFeatures.descriptorBindingVariableDescriptorCount &&
        optionalFeatures.runtimeDescriptorArray &&
        optionalFeatures.descriptorBindingPartiallyBound &&
        optionalFeatures.descriptorBindingSampledImageUpdateAfterBind &&
        optionalFeatures.shaderSampledImageArrayNonUniformIndexing;

    bool supportsDynamicRendering = (optionalFeatures.dynamicRendering || (apiMajor > 1 || apiMinor >= 3));

    _capabilities.platform = ERHIPlatform::Vulkan;
    _capabilities.apiVersionMajor = apiMajor;
    _capabilities.apiVersionMinor = apiMinor;
    _capabilities.apiVersionPatch = properties.apiVersion & 0xFFF;
    _capabilities.deviceName = properties.deviceName;
    _capabilities.supportsDynamicRendering = supportsDynamicRendering;
    _capabilities.renderingPath = supportsDynamicRendering ? ERHIRenderingPath::DynamicRendering : ERHIRenderingPath::LegacyRenderPass;
    _capabilities.supportsBindless = supportsBindless;
    _capabilities.resourceBindingTier = supportsBindless ? ERHIResourceBindingTier::Bindless : ERHIResourceBindingTier::LegacyDescriptorSet;
    _capabilities.maxBindlessSampledImages = properties.limits.maxPerStageDescriptorSampledImages;
    _capabilities.maxBindlessSamplers = properties.limits.maxPerStageDescriptorSamplers;
    _capabilities.maxBindlessStorageImages = properties.limits.maxPerStageDescriptorStorageImages;
    _capabilities.maxBindlessStorageBuffers = properties.limits.maxPerStageDescriptorStorageBuffers;
}

bool VulkanDevice::hasDeviceExtension(const char* extensionName) const
{
    for (const std::string& extension : availableDeviceExtensions)
    {
        if (extension == extensionName)
        {
            return true;
        }
    }
    return false;
}

HS_NS_END
