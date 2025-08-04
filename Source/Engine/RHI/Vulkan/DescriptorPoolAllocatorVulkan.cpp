#include "Engine/RHI/Vulkan/DescriptorPoolAllocatorVulkan.h"

#include "Engine/RHI/Vulkan/RHIDeviceVulkan.h"


HS_NS_BEGIN

DescriptorPoolAllocatorVulkan::DescriptorPoolAllocatorVulkan()
	: _instanceVk(nullptr)
	, _device(nullptr)
	, _setsPerPool(0)
{
	
}
DescriptorPoolAllocatorVulkan::~DescriptorPoolAllocatorVulkan()
{
	Finalize();

	for (auto& p : _readyPools)
	{
		vkDestroyDescriptorPool(_device->logicalDevice, p, nullptr);
		p = nullptr;
	}
	_readyPools.clear();
}


bool DescriptorPoolAllocatorVulkan::Initialize(VkInstance instanceVk, RHIDeviceVulkan* device, uint32 maxSets, const std::vector<PoolSizeRatio>& poolRatios)
{
	_instanceVk = instanceVk;
	_device = device;
	_ratios.clear();
	_setsPerPool = maxSets;

	for (auto& ratio : poolRatios)
	{
		if (ratio.ratio > 0.0f)
		{
			_ratios.push_back(ratio);
		}
	}

	return true;
}

void DescriptorPoolAllocatorVulkan::Finalize()
{
	for (auto& p : _readyPools)
	{
		vkResetDescriptorPool(_device->logicalDevice, p, 0);
	}
	for (auto& p : _fullPools)
	{
		vkResetDescriptorPool(_device->logicalDevice, p, 0);
		_readyPools.push_back(p);
	}
	_fullPools.clear();
}

VkDescriptorSet DescriptorPoolAllocatorVulkan::AllocateDescriptorSet(const VkDescriptorSetLayout& layout, void* next)
{
	VkDescriptorPool pool = acquirePool();

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = next;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkDescriptorSet descriptorSet;
	VkResult result = vkAllocateDescriptorSets(_device->logicalDevice, &allocInfo, &descriptorSet);
	if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
	{

		_fullPools.push_back(pool);

		pool = acquirePool();
		allocInfo.descriptorPool = pool;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(_device->logicalDevice, &allocInfo, &descriptorSet));
	}

	_readyPools.push_back(pool);

	return descriptorSet;
}

void DescriptorPoolAllocatorVulkan::FreeDescriptorSet(VkDescriptorSet set)
{
	if (set == VK_NULL_HANDLE)
	{
		return;
	}
	// Reset the descriptor set
	vkFreeDescriptorSets(_device->logicalDevice, _readyPools.back(), 1, &set);
	// Add the pool back to the ready pools
	if (!_readyPools.empty())
	{
		vkResetDescriptorPool(_device->logicalDevice, _readyPools.back(), 0);
	}
	else
	{
		vkResetDescriptorPool(_device->logicalDevice, _fullPools.back(), 0);
	}
}

VkDescriptorPool DescriptorPoolAllocatorVulkan::acquirePool()
{
	VkDescriptorPool pool;
	if (false == _readyPools.empty())
	{
		pool = _readyPools.back();
		_readyPools.pop_back();
	}
	else
	{
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (const PoolSizeRatio& r : _ratios)
		{
			poolSizes.push_back({ r.type,static_cast<uint32>(r.ratio * _setsPerPool) });
		}

		VkDescriptorPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.pNext = nullptr;
		poolCreateInfo.flags = 0;
		poolCreateInfo.maxSets = _setsPerPool;
		poolCreateInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
		poolCreateInfo.pPoolSizes = poolSizes.data();

		vkCreateDescriptorPool(_device->logicalDevice, &poolCreateInfo, nullptr, &pool);

		_setsPerPool *= 1.5f;
		if (_setsPerPool > 4092)
		{
			_setsPerPool = 4092; // custom limit
		}
	}

	return pool;
}



HS_NS_END