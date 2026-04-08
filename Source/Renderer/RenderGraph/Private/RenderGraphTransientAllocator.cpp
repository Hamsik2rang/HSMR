#include "Renderer/RenderGraph/RenderGraphTransientAllocator.h"

#include <algorithm>

HS_NS_BEGIN

static constexpr uint64 s_defaultHeapSize = 128ull * 1024ull * 1024ull;
static constexpr uint64 s_heapAlignment   = 64ull * 1024ull;

RenderGraphTransientAllocator::~RenderGraphTransientAllocator()
{
    Shutdown();
}

void RenderGraphTransientAllocator::Initialize(RHIContext* rhiContext)
{
    _rhiContext = rhiContext;
}

void RenderGraphTransientAllocator::Shutdown()
{
    reset();
    _rhiContext = nullptr;
}

void RenderGraphTransientAllocator::BeginFrame(uint8 frameIndex)
{
    _frameIndex = frameIndex % static_cast<uint8>(_frameTextures.size());

    for (RHITexture* texture : _frameTextures[_frameIndex])
    {
        destroyTexture(texture);
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

RHITexture* RenderGraphTransientAllocator::CreateTexture(const char* name, const TextureInfo& info, int firstPassIndex, int lastPassIndex)
{
    if (_rhiContext == nullptr || info.isSwapchainTexture)
    {
        return nullptr;
    }

    RHITextureMemoryRequirements requirements = _rhiContext->GetTextureMemoryRequirements(info);
    if (!requirements.isValid || requirements.size == 0)
    {
        return nullptr;
    }

    uint32 heapIndex = 0;
    uint64 heapOffset = 0;
    if (!findPlacement(requirements.memoryTypeIndex, requirements.size, requirements.alignment,
            firstPassIndex, lastPassIndex, heapIndex, heapOffset))
    {
        heapIndex = createHeap(requirements.memoryTypeIndex, requirements.size);
        if (heapIndex == static_cast<uint32>(-1) ||
            !findPlacement(requirements.memoryTypeIndex, requirements.size, requirements.alignment,
                firstPassIndex, lastPassIndex, heapIndex, heapOffset))
        {
            return nullptr;
        }
    }

    Heap& heap = _heaps[heapIndex];
    RHITexture* texture = _rhiContext->CreateTexture(name, info, heap.handle, heapOffset);
    if (texture == nullptr)
    {
        return nullptr;
    }

    heap.ranges.push_back(HeapRange{heapOffset, requirements.size, firstPassIndex, lastPassIndex, _frameIndex});
    _frameTextures[_frameIndex].push_back(texture);
    return texture;
}

uint64 RenderGraphTransientAllocator::alignUp(uint64 value, uint64 alignment) const
{
    if (alignment == 0)
    {
        return value;
    }
    return (value + alignment - 1) & ~(alignment - 1);
}

bool RenderGraphTransientAllocator::overlapsLifetime(const HeapRange& range, int firstPassIndex, int lastPassIndex) const
{
    if (range.frameIndex != _frameIndex)
    {
        return true;
    }
    return !(lastPassIndex < range.firstPassIndex || firstPassIndex > range.lastPassIndex);
}

bool RenderGraphTransientAllocator::findPlacement(uint32 memoryTypeIndex, uint64 size, uint64 alignment,
    int firstPassIndex, int lastPassIndex, uint32& outHeapIndex, uint64& outOffset)
{
    for (uint32 heapIndex = 0; heapIndex < _heaps.size(); heapIndex++)
    {
        Heap& heap = _heaps[heapIndex];
        if (heap.info.memoryTypeIndex != memoryTypeIndex)
        {
            continue;
        }

        uint64 candidate = 0;
        while (candidate + size <= heap.info.size)
        {
            candidate = alignUp(candidate, alignment);
            if (candidate + size > heap.info.size)
            {
                break;
            }

            bool conflict = false;
            uint64 nextCandidate = candidate + alignment;
            for (const HeapRange& range : heap.ranges)
            {
                if (!overlapsLifetime(range, firstPassIndex, lastPassIndex))
                {
                    continue;
                }

                const bool overlapsMemory = !(candidate + size <= range.offset || candidate >= range.offset + range.size);
                if (overlapsMemory)
                {
                    conflict      = true;
                    nextCandidate = alignUp(range.offset + range.size, alignment);
                    break;
                }
            }

            if (!conflict)
            {
                outHeapIndex = heapIndex;
                outOffset    = candidate;
                return true;
            }
            candidate = nextCandidate;
        }
    }

    return false;
}

uint32 RenderGraphTransientAllocator::createHeap(uint32 memoryTypeIndex, uint64 minimumSize)
{
    if (_rhiContext == nullptr)
    {
        return static_cast<uint32>(-1);
    }

    RHIHeapInfo heapInfo{};
    heapInfo.size            = minimumSize > s_defaultHeapSize ? alignUp(minimumSize, s_heapAlignment) : s_defaultHeapSize;
    heapInfo.memoryTypeIndex = memoryTypeIndex;

    RHIHeap* heapHandle = _rhiContext->CreateHeap(heapInfo);
    if (heapHandle == nullptr)
    {
        return static_cast<uint32>(-1);
    }

    Heap heap{};
    heap.handle = heapHandle;
    heap.info   = heapInfo;
    _heaps.push_back(heap);
    return static_cast<uint32>(_heaps.size() - 1);
}

void RenderGraphTransientAllocator::destroyTexture(RHITexture* texture)
{
    if (texture == nullptr || _rhiContext == nullptr)
    {
        return;
    }
    _rhiContext->DestroyTexture(texture);
}

void RenderGraphTransientAllocator::reset()
{
    if (_rhiContext == nullptr)
    {
        for (std::vector<RHITexture*>& frameTextures : _frameTextures)
        {
            frameTextures.clear();
        }
        _heaps.clear();
        return;
    }

    for (std::vector<RHITexture*>& frameTextures : _frameTextures)
    {
        for (RHITexture* texture : frameTextures)
        {
            destroyTexture(texture);
        }
        frameTextures.clear();
    }

    for (Heap& heap : _heaps)
    {
        if (heap.handle != nullptr)
        {
            _rhiContext->DestroyHeap(heap.handle);
            heap.handle = nullptr;
        }
        heap.ranges.clear();
    }
    _heaps.clear();
}

HS_NS_END
