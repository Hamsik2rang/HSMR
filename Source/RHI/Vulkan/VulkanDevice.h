#ifndef __HS_RHI_DEVICE_VULKAN_H__
#define __HS_RHI_DEVICE_VULKAN_H__

#include "Precompile.h"

#include "RHI/Vulkan/VulkanUtility.h"

#include <string>
#include <vector>

HS_NS_BEGIN

struct VulkanOptionalDeviceExtension
{
    union
    {
        struct
        {
            // Optional Extensions
            uint64 hasEXTValidationCache           : 1;
            uint64 hasMemoryPriority               : 1;
            uint64 hasMemoryBudget                 : 1;
            uint64 hasEXTASTCDecodeMode            : 1;
            uint64 hasEXTFragmentDensityMap        : 1;
            uint64 hasEXTFragmentDensityMap2       : 1;
            uint64 hasKHRFragmentShadingRate       : 1;
            uint64 hasKHRFragmentShaderBarycentric : 1;
            uint64 hasEXTFullscreenExclusive       : 1;
            uint64 hasImageAtomicInt64             : 1;
            uint64 hasAccelerationStructure        : 1;
            uint64 hasRayTracingPipeline           : 1;
            uint64 hasRayQuery                     : 1;
            uint64 hasKHRPipelineLibrary           : 1;
            uint64 hasDeferredHostOperations       : 1;
            uint64 hasEXTCalibratedTimestamps      : 1;
            uint64 hasEXTDescriptorBuffer          : 1;
            uint64 hasEXTDeviceFault               : 1;
            uint64 hasEXTMeshShader                : 1;
            uint64 hasEXTToolingInfo               : 1;
            uint64 hasEXTImageCompressionControl   : 1;
            uint64 hasEXTMutableDescriptorType     : 1;
            uint64 hasKHRMaintenance7              : 1;
            uint64 hasEXTShaderObject              : 1;
            uint64 hasEXTGraphicsPipelineLibrary   : 1;

            // Vendor specific
            uint64 hasAMDBufferMarker                     : 1;
            uint64 hasNVDiagnosticCheckpoints             : 1;
            uint64 hasNVDeviceDiagnosticConfig            : 1;
            uint64 hasANDROIDExternalMemoryHardwareBuffer : 1;

            // Promoted to 1.1
            uint64 hasKHRMultiview              : 1;
            uint64 hasKHR16bitStorage           : 1;
            uint64 hasKHRSamplerYcbcrConversion : 1;

            // Promoted to 1.2
            uint64 hasKHRRenderPass2              : 1;
            uint64 hasKHRImageFormatList          : 1;
            uint64 hasKHRShaderAtomicInt64        : 1;
            uint64 hasEXTScalarBlockLayout        : 1;
            uint64 hasBufferDeviceAddress         : 1;
            uint64 hasSPIRV_14                    : 1;
            uint64 hasShaderFloatControls         : 1;
            uint64 hasKHRShaderFloat16            : 1;
            uint64 hasEXTDescriptorIndexing       : 1;
            uint64 hasSeparateDepthStencilLayouts : 1;
            uint64 hasEXTHostQueryReset           : 1;
            uint64 hasQcomRenderPassShaderResolve : 1;
            uint64 hasKHRDepthStencilResolve      : 1;
            uint64 hasKHRTimelineSemaphore        : 1;

            // Promoted to 1.3
            uint64 hasEXTTextureCompressionASTCHDR    : 1;
            uint64 hasKHRMaintenance4                 : 1;
            uint64 hasKHRMaintenance5                 : 1;
            uint64 hasKHRSynchronization2             : 1;
            uint64 hasKHRDynamicRendering             : 1;
            uint64 hasEXTSubgroupSizeControl          : 1;
            uint64 hasEXTPipelineCreationCacheControl : 1;
            uint64 hasEXTExtendedDynamicState1        : 1;
            uint64 hasEXTExtendedDynamicState2        : 1;
            uint64 hasEXTExtendedDynamicState3        : 1;
            uint64 hasEXTVertexInputDynamicState      : 1;

            // Promoted to 1.4
            uint64 hasEXTLoadStoreOpNone : 1;
            uint64 hasEXTHostImageCopy   : 1;
        };
        uint64 packed = 0;
    };
};

class HS_RHI_API VulkanDevice final
{
public:
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
        uint32 graphics;
        uint32 compute;
        uint32 transfer;
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
    RHICapabilities capabilities;

    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    VkCommandPool commandPool = VK_NULL_HANDLE;

	HS_FORCEINLINE operator VkDevice() { return logicalDevice;  }
    HS_FORCEINLINE const RHICapabilities& GetCapabilities() const { return capabilities; }

private:
	void getPhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	uint32 getPhysicalDeviceScore(VkPhysicalDevice device);
    void queryDeviceCapabilities(VkPhysicalDevice device);
    bool hasDeviceExtension(const char* extensionName) const;

    VulkanOptionalDeviceExtension _optionalExtensions;
    VkInstance _instanceVk;
};

HS_NS_END



#endif
