//
//  MemoryPool.h
//  Core
//
//  High-performance memory allocators for HSMR Engine
//  Reduces memory fragmentation and provides predictable allocation performance
//
#ifndef __HS_CORE_MEMORY_POOL_H__
#define __HS_CORE_MEMORY_POOL_H__

#include "Precompile.h"
#include "Core/Log.h"

#include <atomic>
#include <array>

HS_NS_BEGIN

// Forward declarations
class LinearAllocator;
class StackAllocator;

// Platform-specific aligned allocation utilities
namespace MemoryUtils
{
    inline void* AlignedAlloc(size_t size, size_t alignment) noexcept
    {
        #if defined(_WIN32)
            return _aligned_malloc(size, alignment);
        #elif defined(__APPLE__) || defined(__linux__)
            void* ptr = nullptr;
            if (posix_memalign(&ptr, alignment, size) != 0)
                return nullptr;
            return ptr;
        #else
            // Fallback: manual alignment
            void* ptr = std::malloc(size + alignment + sizeof(void*));
            if (!ptr) return nullptr;
            
            uintptr_t aligned = (reinterpret_cast<uintptr_t>(ptr) + sizeof(void*) + alignment - 1) & ~(alignment - 1);
            void** header = reinterpret_cast<void**>(aligned - sizeof(void*));
            *header = ptr;
            return reinterpret_cast<void*>(aligned);
        #endif
    }

    inline void AlignedFree(void* ptr) noexcept
    {
        if (!ptr) return;
        
        #if defined(_WIN32)
            _aligned_free(ptr);
        #elif defined(__APPLE__) || defined(__linux__)
            std::free(ptr);
        #else
            // Fallback: retrieve original pointer
            void** header = reinterpret_cast<void**>(ptr) - 1;
            std::free(*header);
        #endif
    }
}

// Default allocator interface - can be swapped for different strategies
class DefaultAllocator
{
public:
    static constexpr size_t ALIGNMENT = alignof(std::max_align_t);

    void* Allocate(size_t size, size_t alignment = ALIGNMENT)
    {
        // For debug builds, use system allocator to catch issues
        #ifdef _DEBUG
            return MemoryUtils::AlignedAlloc(size, alignment);
        #else
            // Use our high-performance pool in release builds
            return GetGlobalPool().Allocate(size, alignment);
        #endif
    }

    void Deallocate(void* ptr, size_t size) noexcept
    {
        #ifdef _DEBUG
            MemoryUtils::AlignedFree(ptr);
        #else
            GetGlobalPool().Deallocate(ptr, size);
        #endif
    }

    bool operator==(const DefaultAllocator&) const noexcept { return true; }
    bool operator!=(const DefaultAllocator&) const noexcept { return false; }

private:
    static LinearAllocator& GetGlobalPool();
};

// Linear allocator - very fast, no individual deallocation
class LinearAllocator
{
private:
    uint8* _buffer;
    size_t _size;
    size_t _offset;
    bool _ownBuffer;

public:
    LinearAllocator(size_t size)
        : _size(size)
        , _offset(0)
        , _ownBuffer(true)
    {
        _buffer = static_cast<uint8*>(MemoryUtils::AlignedAlloc(size, 64)); // 64-byte aligned
        HS_ASSERT(_buffer != nullptr, "Failed to allocate linear buffer");
    }

    LinearAllocator(void* buffer, size_t size)
        : _buffer(static_cast<uint8*>(buffer))
        , _size(size)
        , _offset(0)
        , _ownBuffer(false)
    {
    }

    ~LinearAllocator()
    {
        if (_ownBuffer && _buffer)
        {
            MemoryUtils::AlignedFree(_buffer);
        }
    }

    // Non-copyable
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;

    // Moveable
    LinearAllocator(LinearAllocator&& other) noexcept
        : _buffer(other._buffer)
        , _size(other._size)
        , _offset(other._offset)
        , _ownBuffer(other._ownBuffer)
    {
        other._buffer = nullptr;
        other._ownBuffer = false;
    }

    LinearAllocator& operator=(LinearAllocator&& other) noexcept
    {
        if (this != &other)
        {
            if (_ownBuffer && _buffer)
            {
                MemoryUtils::AlignedFree(_buffer);
            }

            _buffer = other._buffer;
            _size = other._size;
            _offset = other._offset;
            _ownBuffer = other._ownBuffer;

            other._buffer = nullptr;
            other._ownBuffer = false;
        }
        return *this;
    }

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        // Align the offset
        size_t alignedOffset = (_offset + alignment - 1) & ~(alignment - 1);
        
        if (alignedOffset + size > _size)
        {
            HS_LOG(crash, "Linear allocator out of memory: requested %zu, available %zu", 
                   size, _size - alignedOffset);
            return nullptr;
        }

        void* ptr = _buffer + alignedOffset;
        _offset = alignedOffset + size;
        return ptr;
    }

    void Deallocate(void* ptr, size_t size) noexcept
    {
        // Linear allocator doesn't support individual deallocation
        // This is intentional for performance
    }

    void Reset() noexcept
    {
        _offset = 0;
    }

    size_t GetUsedSize() const noexcept { return _offset; }
    size_t GetTotalSize() const noexcept { return _size; }
    size_t GetFreeSize() const noexcept { return _size - _offset; }
    float GetUsagePercentage() const noexcept { return static_cast<float>(_offset) / _size * 100.0f; }
};

// Pool allocator for fixed-size objects
template<size_t BlockSize, size_t BlockCount>
class PoolAllocator
{
private:
    struct FreeNode
    {
        FreeNode* next;
    };

    alignas(64) uint8 _memory[BlockSize * BlockCount];  // Cache-line aligned
    FreeNode* _freeHead;
    std::atomic<uint32> _allocatedCount;

public:
    PoolAllocator()
        : _freeHead(nullptr)
        , _allocatedCount(0)
    {
        // Initialize free list
        _freeHead = reinterpret_cast<FreeNode*>(_memory);
        FreeNode* current = _freeHead;

        for (size_t i = 0; i < BlockCount - 1; ++i)
        {
            current->next = reinterpret_cast<FreeNode*>(_memory + (i + 1) * BlockSize);
            current = current->next;
        }
        current->next = nullptr;
    }

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        if (size > BlockSize)
        {
            HS_LOG(error, "Requested size %zu exceeds block size %zu", size, static_cast<size_t>(BlockSize));
            return nullptr;
        }

        if (!_freeHead)
        {
            HS_LOG(error, "Pool allocator exhausted");
            return nullptr;
        }

        void* ptr = _freeHead;
        _freeHead = _freeHead->next;
        _allocatedCount.fetch_add(1, std::memory_order_relaxed);

        return ptr;
    }

    void Deallocate(void* ptr, size_t size) noexcept
    {
        if (!ptr || ptr < _memory || ptr >= _memory + sizeof(_memory))
        {
            return; // Invalid pointer
        }

        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = _freeHead;
        _freeHead = node;
        _allocatedCount.fetch_sub(1, std::memory_order_relaxed);
    }

    size_t GetAllocatedCount() const noexcept 
    { 
        return _allocatedCount.load(std::memory_order_relaxed); 
    }
    
    size_t GetFreeCount() const noexcept 
    { 
        return BlockCount - GetAllocatedCount(); 
    }
    
    bool IsFull() const noexcept 
    { 
        return GetAllocatedCount() >= BlockCount; 
    }
    
    bool IsEmpty() const noexcept 
    { 
        return GetAllocatedCount() == 0; 
    }
};

// Stack allocator - LIFO allocation, very fast
class StackAllocator
{
private:
    struct Marker
    {
        size_t offset;
        size_t prevMarker;
    };

    uint8* _buffer;
    size_t _size;
    size_t _offset;
    size_t _markerOffset;
    bool _ownBuffer;

public:
    StackAllocator(size_t size)
        : _size(size)
        , _offset(0)
        , _markerOffset(0)
        , _ownBuffer(true)
    {
        _buffer = static_cast<uint8*>(MemoryUtils::AlignedAlloc(size, 64));
        HS_ASSERT(_buffer != nullptr, "Failed to allocate stack buffer");
    }

    ~StackAllocator()
    {
        if (_ownBuffer && _buffer)
        {
            MemoryUtils::AlignedFree(_buffer);
        }
    }

    // Non-copyable, moveable
    StackAllocator(const StackAllocator&) = delete;
    StackAllocator& operator=(const StackAllocator&) = delete;
    
    StackAllocator(StackAllocator&& other) noexcept
        : _buffer(other._buffer)
        , _size(other._size)
        , _offset(other._offset)
        , _markerOffset(other._markerOffset)
        , _ownBuffer(other._ownBuffer)
    {
        other._buffer = nullptr;
        other._ownBuffer = false;
    }

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        size_t alignedOffset = (_offset + alignment - 1) & ~(alignment - 1);
        
        if (alignedOffset + size > _size)
        {
            return nullptr;
        }

        void* ptr = _buffer + alignedOffset;
        _offset = alignedOffset + size;
        return ptr;
    }

    void Deallocate(void* ptr, size_t size) noexcept
    {
        // Stack allocator only supports LIFO deallocation
        // Check if this is the top allocation
        if (static_cast<uint8*>(ptr) + size == _buffer + _offset)
        {
            _offset = static_cast<uint8*>(ptr) - _buffer;
        }
    }

    // Marker-based allocation management
    size_t PushMarker()
    {
        if (_offset + sizeof(Marker) > _size)
            return SIZE_MAX; // Invalid marker

        Marker* marker = reinterpret_cast<Marker*>(_buffer + _offset);
        marker->offset = _offset;
        marker->prevMarker = _markerOffset;
        
        _markerOffset = _offset;
        _offset += sizeof(Marker);
        
        return _markerOffset;
    }

    void PopToMarker(size_t markerPos)
    {
        if (markerPos == SIZE_MAX || markerPos > _offset)
            return;

        Marker* marker = reinterpret_cast<Marker*>(_buffer + markerPos);
        _offset = marker->offset;
        _markerOffset = marker->prevMarker;
    }

    void Reset() noexcept
    {
        _offset = 0;
        _markerOffset = 0;
    }

    size_t GetUsedSize() const noexcept { return _offset; }
    size_t GetFreeSize() const noexcept { return _size - _offset; }
};

// Thread-local allocators for per-frame allocations
class FrameAllocator
{
private:
    static constexpr size_t FRAME_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB per frame
    
    thread_local static std::unique_ptr<LinearAllocator> t_currentFrame;
    thread_local static std::unique_ptr<LinearAllocator> t_previousFrame;

public:
    static void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        if (!t_currentFrame)
        {
            t_currentFrame = std::make_unique<LinearAllocator>(FRAME_BUFFER_SIZE);
        }
        return t_currentFrame->Allocate(size, alignment);
    }

    static void Deallocate(void* ptr, size_t size) noexcept
    {
        // Frame allocator doesn't support individual deallocation
    }

    // Call at the end of each frame
    static void SwapFrames()
    {
        if (!t_currentFrame)
        {
            t_currentFrame = std::make_unique<LinearAllocator>(FRAME_BUFFER_SIZE);
        }
        if (!t_previousFrame)
        {
            t_previousFrame = std::make_unique<LinearAllocator>(FRAME_BUFFER_SIZE);
        }

        // Swap and reset
        std::swap(t_currentFrame, t_previousFrame);
        t_currentFrame->Reset();
    }

    static size_t GetCurrentFrameUsage()
    {
        return t_currentFrame ? t_currentFrame->GetUsedSize() : 0;
    }
};

// Global memory statistics
struct MemoryStats
{
    std::atomic<size_t> totalAllocated{0};
    std::atomic<size_t> totalDeallocated{0};
    std::atomic<size_t> currentUsage{0};
    std::atomic<size_t> peakUsage{0};
    std::atomic<uint64> allocationCount{0};
    std::atomic<uint64> deallocationCount{0};

    void RecordAllocation(size_t size)
    {
        totalAllocated.fetch_add(size, std::memory_order_relaxed);
        allocationCount.fetch_add(1, std::memory_order_relaxed);
        
        size_t current = currentUsage.fetch_add(size, std::memory_order_relaxed) + size;
        
        // Update peak usage atomically
        size_t peak = peakUsage.load(std::memory_order_relaxed);
        while (current > peak && !peakUsage.compare_exchange_weak(peak, current, std::memory_order_relaxed))
        {
            // Retry if another thread updated peak
        }
    }

    void RecordDeallocation(size_t size)
    {
        totalDeallocated.fetch_add(size, std::memory_order_relaxed);
        deallocationCount.fetch_add(1, std::memory_order_relaxed);
        currentUsage.fetch_sub(size, std::memory_order_relaxed);
    }
};

// Global memory management
class MemoryManager
{
private:
    static std::unique_ptr<LinearAllocator> s_globalPool;
    static MemoryStats s_stats;

public:
    static bool Initialize(size_t poolSize = 256 * 1024 * 1024) // 256MB default
    {
        if (s_globalPool)
            return false; // Already initialized

        s_globalPool = std::make_unique<LinearAllocator>(poolSize);
        return s_globalPool != nullptr;
    }

    static void Shutdown()
    {
        s_globalPool.reset();
    }

    static LinearAllocator* GetGlobalPool() { return s_globalPool.get(); }
    static const MemoryStats& GetStats() { return s_stats; }
    
    static void RecordAllocation(size_t size) { s_stats.RecordAllocation(size); }
    static void RecordDeallocation(size_t size) { s_stats.RecordDeallocation(size); }
};

HS_NS_END

#endif // __HS_CORE_MEMORY_POOL_H__