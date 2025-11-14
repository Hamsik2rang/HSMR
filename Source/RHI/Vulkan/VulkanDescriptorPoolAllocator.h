#ifndef __HS_DESCRIPTOR_POOL_ALLOCATOR_VULKAN_H__
#define __HS_DESCRIPTOR_POOL_ALLOCATOR_VULKAN_H__

#include "Precompile.h"

#include "RHI/Vulkan/VulkanResourceHandle.h"

#include <vector>

HS_NS_BEGIN

class VulkanDevice;

class HS_API DescriptorPoolAllocatorVulkan
{
public:
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};
	
	DescriptorPoolAllocatorVulkan();
	~DescriptorPoolAllocatorVulkan();

	bool Initialize(VkInstance instanceVk, VulkanDevice* device, uint32 maxSets, const std::vector<PoolSizeRatio>& poolRatios);
	void Finalize();

	VkDescriptorSet AllocateDescriptorSet(const VkDescriptorSetLayout& layout, void* next);
	void FreeDescriptorSet(VkDescriptorSet set);

private:
	VkDescriptorPool acquirePool();
	
	VkInstance _instanceVk;
	VulkanDevice* _device = nullptr;

	std::vector<PoolSizeRatio> _ratios;
	std::vector<VkDescriptorPool> _readyPools;
	std::vector<VkDescriptorPool> _fullPools;
	uint32 _setsPerPool;
};

HS_NS_END

#endif