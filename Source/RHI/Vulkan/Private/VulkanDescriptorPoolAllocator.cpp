#include "RHI/Vulkan/VulkanDescriptorPoolAllocator.h"

#include "RHI/Vulkan/VulkanDevice.h"

HS_NS_BEGIN

VulkanDescriptorPoolAllocator::VulkanDescriptorPoolAllocator()
    : _instanceVk(nullptr)
    , _device(nullptr)
    , _setsPerPool(0)
{
}
VulkanDescriptorPoolAllocator::~VulkanDescriptorPoolAllocator()
{
    Finalize();
}

bool VulkanDescriptorPoolAllocator::Initialize(VkInstance instanceVk, VulkanDevice* device, uint32 maxSets, const std::vector<PoolSizeRatio>& poolRatios)
{
    _instanceVk = instanceVk;
    _device     = device;
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

void VulkanDescriptorPoolAllocator::Finalize()
{
    if (_device == nullptr || _device->logicalDevice == VK_NULL_HANDLE)
    {
        _readyPools.clear();
        _fullPools.clear();
        return;
    }

    for (auto& p : _readyPools)
    {
        if (p != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(_device->logicalDevice, p, nullptr);
            p = VK_NULL_HANDLE;
        }
    }
    for (auto& p : _fullPools)
    {
        if (p != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(_device->logicalDevice, p, nullptr);
            p = VK_NULL_HANDLE;
        }
    }
    _readyPools.clear();
    _fullPools.clear();
}

VkDescriptorSet VulkanDescriptorPoolAllocator::AllocateDescriptorSet(const VkDescriptorSetLayout& layout, void* next,
                                                                       VkDescriptorPool& outPool)
{
    VkDescriptorPool pool = acquirePool();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext              = next;
    allocInfo.descriptorPool     = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &layout;

    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(_device->logicalDevice, &allocInfo, &descriptorSet);
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
    {
        _fullPools.push_back(pool);

        pool                     = acquirePool();
        allocInfo.descriptorPool = pool;

        VK_CHECK_RESULT(vkAllocateDescriptorSets(_device->logicalDevice, &allocInfo, &descriptorSet));
    }

    _readyPools.push_back(pool);
    outPool = pool;

    return descriptorSet;
}

void VulkanDescriptorPoolAllocator::FreeDescriptorSet(VkDescriptorSet set, VkDescriptorPool pool)
{
    if (set == VK_NULL_HANDLE || pool == VK_NULL_HANDLE) return;
    vkFreeDescriptorSets(_device->logicalDevice, pool, 1, &set);
    // vkResetDescriptorPool을 호출하지 않습니다.
    // 풀 전체 정리는 Finalize()에서만 수행합니다.
}

VkDescriptorPool VulkanDescriptorPoolAllocator::acquirePool()
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
            poolSizes.push_back({r.type, static_cast<uint32>(r.ratio * _setsPerPool)});
        }

        VkDescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.pNext         = nullptr;
        poolCreateInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolCreateInfo.maxSets       = _setsPerPool;
        poolCreateInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
        poolCreateInfo.pPoolSizes    = poolSizes.data();

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
