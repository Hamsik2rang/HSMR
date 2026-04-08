#ifndef __HS_RHI_VULKAN_TRANSIENT_RESOURCE_ALLOCATOR_H__
#define __HS_RHI_VULKAN_TRANSIENT_RESOURCE_ALLOCATOR_H__

#include "Precompile.h"

#include "RHI/TransientResourceAllocator.h"
#include "RHI/Vulkan/VulkanDevice.h"

#include <array>

HS_NS_BEGIN

class HS_RHI_API VulkanTransientResourceAllocator final : public RHITransientResourceAllocator
{
public:
    VulkanTransientResourceAllocator() = default;
    ~VulkanTransientResourceAllocator() override;

    void Initialize(VulkanDevice* device);
    void Finalize();

    void BeginFrame(uint8 frameIndex) override;
    RHITexture* CreateTexture(const char* name, const TextureInfo& info, int firstPassIndex, int lastPassIndex) override;
    void ReleaseTexture(RHITexture* texture) override;
    void Reset() override;
    bool IsSupported() const override { return _device != nullptr && _device->logicalDevice != VK_NULL_HANDLE; }

private:
    struct HeapRange
    {
        VkDeviceSize offset = 0;
        VkDeviceSize size   = 0;
        int firstPassIndex  = -1;
        int lastPassIndex   = -1;
        uint8 frameIndex    = 0;
    };

    struct Heap
    {
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size     = 0;
        uint32 memoryTypeIndex = 0;
        std::vector<HeapRange> ranges;
    };

    uint32 getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties) const;
    VkDeviceSize alignUp(VkDeviceSize value, VkDeviceSize alignment) const;
    bool overlapsLifetime(const HeapRange& range, int firstPassIndex, int lastPassIndex) const;
    bool findPlacement(uint32 memoryTypeIndex, VkDeviceSize size, VkDeviceSize alignment,
                       int firstPassIndex, int lastPassIndex, uint32& outHeapIndex, VkDeviceSize& outOffset);
    uint32 createHeap(uint32 memoryTypeIndex, VkDeviceSize minimumSize);
    void destroyTextureInternal(class VulkanTexture* texture);

    VulkanDevice* _device = nullptr;
    uint8 _frameIndex = 0;

    std::vector<Heap> _heaps;
    std::array<std::vector<class VulkanTexture*>, 2> _frameTextures;
};

HS_NS_END

#endif
