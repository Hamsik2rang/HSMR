#include "RHI/Vulkan/VulkanTransientResourceAllocator.h"

#include "RHI/Vulkan/VulkanResourceHandle.h"
#include "RHI/Vulkan/VulkanUtility.h"

#include <algorithm>

HS_NS_BEGIN

static constexpr VkDeviceSize s_defaultHeapSize = 128ull * 1024ull * 1024ull;

VulkanTransientResourceAllocator::~VulkanTransientResourceAllocator()
{
    Finalize();
}

void VulkanTransientResourceAllocator::Initialize(VulkanDevice* device)
{
    _device = device;
}

void VulkanTransientResourceAllocator::Finalize()
{
    Reset();
    _device = nullptr;
}

void VulkanTransientResourceAllocator::BeginFrame(uint8 frameIndex)
{
    _frameIndex = frameIndex % static_cast<uint8>(_frameTextures.size());
    for (VulkanTexture* texture : _frameTextures[_frameIndex])
    {
        destroyTextureInternal(texture);
    }
    _frameTextures[_frameIndex].clear();

    for (Heap& heap : _heaps)
    {
        heap.ranges.erase(
            std::remove_if(heap.ranges.begin(), heap.ranges.end(),
                [this](const HeapRange& range)
                {
                    return range.frameIndex == _frameIndex;
                }),
            heap.ranges.end());
    }
}

RHITexture* VulkanTransientResourceAllocator::CreateTexture(const char* name, const TextureInfo& info, int firstPassIndex, int lastPassIndex)
{
    HS_ASSERT(IsSupported(), "Vulkan transient allocator is not initialized");
    HS_ASSERT(info.isSwapchainTexture == false, "Swapchain textures cannot use transient heaps");

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = RHIUtilityVulkan::ToImageType(info.type);
    imageCreateInfo.format        = RHIUtilityVulkan::ToPixelFormat(info.format);
    imageCreateInfo.usage         = RHIUtilityVulkan::ToTextureUsage(info.usage);
    imageCreateInfo.extent.width  = info.extent.width;
    imageCreateInfo.extent.height = info.extent.height;
    imageCreateInfo.extent.depth  = (info.type == ETextureType::Tex3D) ? info.extent.depth : 1;
    imageCreateInfo.arrayLayers   = info.type == ETextureType::TexCube ? 6 : 1;
    imageCreateInfo.mipLevels     = 1;
    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.flags         = VK_IMAGE_CREATE_ALIAS_BIT;
    if (info.type == ETextureType::TexCube)
    {
        imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VkImage imageVk = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateImage(_device->logicalDevice, &imageCreateInfo, nullptr, &imageVk));

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(_device->logicalDevice, imageVk, &memoryRequirements);

    uint32 memoryTypeIndex = getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    uint32 heapIndex = 0;
    VkDeviceSize heapOffset = 0;
    if (!findPlacement(memoryTypeIndex, memoryRequirements.size, memoryRequirements.alignment,
                       firstPassIndex, lastPassIndex, heapIndex, heapOffset))
    {
        heapIndex = createHeap(memoryTypeIndex, memoryRequirements.size);
        bool placed = findPlacement(memoryTypeIndex, memoryRequirements.size, memoryRequirements.alignment,
                                    firstPassIndex, lastPassIndex, heapIndex, heapOffset);
        HS_ASSERT(placed, "Failed to place transient texture in a fresh heap");
    }

    Heap& heap = _heaps[heapIndex];
    VK_CHECK_RESULT(vkBindImageMemory(_device->logicalDevice, imageVk, heap.memory, heapOffset));
    heap.ranges.push_back(HeapRange{heapOffset, memoryRequirements.size, firstPassIndex, lastPassIndex, _frameIndex});

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    if (info.isDepthStencilBuffer)
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (info.format == EPixelFormat::Depth24Stencil8 || info.format == EPixelFormat::Depth32Stencil8)
        {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = imageVk;
    viewCreateInfo.viewType = RHIUtilityVulkan::ToImageViewType(info.type);
    viewCreateInfo.format = imageCreateInfo.format;
    viewCreateInfo.subresourceRange.aspectMask = aspectMask;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = imageCreateInfo.mipLevels;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = imageCreateInfo.arrayLayers;

    VkImageView imageViewVk = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateImageView(_device->logicalDevice, &viewCreateInfo, nullptr, &imageViewVk));

    VulkanTexture* textureVK = new VulkanTexture(name, info);
    textureVK->handle = imageVk;
    textureVK->imageViewVk = imageViewVk;
    textureVK->memoryVk = heap.memory;
    textureVK->layoutVk = VK_IMAGE_LAYOUT_UNDEFINED;
    textureVK->memoryOffset = heapOffset;
    textureVK->memorySize = memoryRequirements.size;
    textureVK->transientHeapIndex = heapIndex;
    textureVK->transientFrameIndex = _frameIndex;
    textureVK->ownsMemory = false;
    textureVK->isTransient = true;

    _frameTextures[_frameIndex].push_back(textureVK);
    return textureVK;
}

void VulkanTransientResourceAllocator::ReleaseTexture(RHITexture* texture)
{
    VulkanTexture* textureVK = static_cast<VulkanTexture*>(texture);
    if (textureVK == nullptr)
    {
        return;
    }

    for (std::vector<VulkanTexture*>& frameTextures : _frameTextures)
    {
        auto it = std::find(frameTextures.begin(), frameTextures.end(), textureVK);
        if (it != frameTextures.end())
        {
            frameTextures.erase(it);
            break;
        }
    }

    if (textureVK->transientHeapIndex < _heaps.size())
    {
        Heap& heap = _heaps[textureVK->transientHeapIndex];
        heap.ranges.erase(
            std::remove_if(heap.ranges.begin(), heap.ranges.end(),
                [textureVK](const HeapRange& range)
                {
                    return range.offset == textureVK->memoryOffset &&
                           range.size == textureVK->memorySize &&
                           range.frameIndex == textureVK->transientFrameIndex;
                }),
            heap.ranges.end());
    }

    destroyTextureInternal(textureVK);
}

void VulkanTransientResourceAllocator::Reset()
{
    if (_device == nullptr || _device->logicalDevice == VK_NULL_HANDLE)
    {
        for (std::vector<VulkanTexture*>& frameTextures : _frameTextures)
        {
            frameTextures.clear();
        }
        _heaps.clear();
        return;
    }

    for (std::vector<VulkanTexture*>& frameTextures : _frameTextures)
    {
        for (VulkanTexture* texture : frameTextures)
        {
            destroyTextureInternal(texture);
        }
        frameTextures.clear();
    }

    for (Heap& heap : _heaps)
    {
        if (heap.memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(_device->logicalDevice, heap.memory, nullptr);
            heap.memory = VK_NULL_HANDLE;
        }
        heap.ranges.clear();
    }
    _heaps.clear();
}

uint32 VulkanTransientResourceAllocator::getMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(_device->physicalDevice, &memoryProperties);
    for (uint32 i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) != 0)
        {
            if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        typeBits >>= 1;
    }
    return 0;
}

VkDeviceSize VulkanTransientResourceAllocator::alignUp(VkDeviceSize value, VkDeviceSize alignment) const
{
    if (alignment == 0)
    {
        return value;
    }
    return (value + alignment - 1) & ~(alignment - 1);
}

bool VulkanTransientResourceAllocator::overlapsLifetime(const HeapRange& range, int firstPassIndex, int lastPassIndex) const
{
    if (range.frameIndex != _frameIndex)
    {
        return true;
    }
    return !(lastPassIndex < range.firstPassIndex || firstPassIndex > range.lastPassIndex);
}

bool VulkanTransientResourceAllocator::findPlacement(uint32 memoryTypeIndex, VkDeviceSize size, VkDeviceSize alignment,
                                                     int firstPassIndex, int lastPassIndex,
                                                     uint32& outHeapIndex, VkDeviceSize& outOffset)
{
    for (uint32 heapIndex = 0; heapIndex < _heaps.size(); heapIndex++)
    {
        Heap& heap = _heaps[heapIndex];
        if (heap.memoryTypeIndex != memoryTypeIndex)
        {
            continue;
        }

        VkDeviceSize candidate = 0;
        while (candidate + size <= heap.size)
        {
            candidate = alignUp(candidate, alignment);
            if (candidate + size > heap.size)
            {
                break;
            }
            bool conflict = false;
            VkDeviceSize nextCandidate = candidate + alignment;
            for (const HeapRange& range : heap.ranges)
            {
                if (!overlapsLifetime(range, firstPassIndex, lastPassIndex))
                {
                    continue;
                }

                bool overlapsMemory = !(candidate + size <= range.offset || candidate >= range.offset + range.size);
                if (overlapsMemory)
                {
                    conflict = true;
                    nextCandidate = alignUp(range.offset + range.size, alignment);
                    break;
                }
            }

            if (!conflict)
            {
                outHeapIndex = heapIndex;
                outOffset = candidate;
                return true;
            }
            candidate = nextCandidate;
        }
    }

    return false;
}

uint32 VulkanTransientResourceAllocator::createHeap(uint32 memoryTypeIndex, VkDeviceSize minimumSize)
{
    Heap heap{};
    heap.size = minimumSize > s_defaultHeapSize ? alignUp(minimumSize, 64ull * 1024ull) : s_defaultHeapSize;
    heap.memoryTypeIndex = memoryTypeIndex;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = heap.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VK_CHECK_RESULT(vkAllocateMemory(_device->logicalDevice, &allocInfo, nullptr, &heap.memory));
    _heaps.push_back(heap);
    return static_cast<uint32>(_heaps.size() - 1);
}

void VulkanTransientResourceAllocator::destroyTextureInternal(VulkanTexture* texture)
{
    if (texture == nullptr || _device == nullptr || _device->logicalDevice == VK_NULL_HANDLE)
    {
        delete texture;
        return;
    }

    if (texture->imageViewVk != VK_NULL_HANDLE)
    {
        vkDestroyImageView(_device->logicalDevice, texture->imageViewVk, nullptr);
        texture->imageViewVk = VK_NULL_HANDLE;
    }
    if (texture->handle != VK_NULL_HANDLE)
    {
        vkDestroyImage(_device->logicalDevice, texture->handle, nullptr);
        texture->handle = VK_NULL_HANDLE;
    }
    delete texture;
}

HS_NS_END
