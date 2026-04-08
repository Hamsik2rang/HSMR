#ifndef __HS_RENDERER_RENDER_GRAPH_TRANSIENT_ALLOCATOR_H__
#define __HS_RENDERER_RENDER_GRAPH_TRANSIENT_ALLOCATOR_H__

#include "Precompile.h"

#include "RHI/RHIContext.h"

#include <array>
#include <vector>

HS_NS_BEGIN

class HS_RENDERER_API RenderGraphTransientAllocator
{
public:
    RenderGraphTransientAllocator() = default;
    ~RenderGraphTransientAllocator();

    void Initialize(RHIContext* rhiContext);
    void Shutdown();
    void BeginFrame(uint8 frameIndex);

    RHITexture* CreateTexture(const char* name, const TextureInfo& info, int firstPassIndex, int lastPassIndex);

private:
    struct HeapRange
    {
        uint64 offset       = 0;
        uint64 size         = 0;
        int firstPassIndex  = -1;
        int lastPassIndex   = -1;
        uint8 frameIndex    = 0;
    };

    struct Heap
    {
        RHIHeap* handle = nullptr;
        RHIHeapInfo info{};
        std::vector<HeapRange> ranges;
    };

    uint64 alignUp(uint64 value, uint64 alignment) const;
    bool overlapsLifetime(const HeapRange& range, int firstPassIndex, int lastPassIndex) const;
    bool findPlacement(uint32 memoryTypeIndex, uint64 size, uint64 alignment,
                       int firstPassIndex, int lastPassIndex, uint32& outHeapIndex, uint64& outOffset);
    uint32 createHeap(uint32 memoryTypeIndex, uint64 minimumSize);
    void destroyTexture(RHITexture* texture);
    void reset();

    RHIContext* _rhiContext = nullptr;
    uint8 _frameIndex       = 0;

    std::vector<Heap> _heaps;
    std::array<std::vector<RHITexture*>, 2> _frameTextures;
};

HS_NS_END

#endif
