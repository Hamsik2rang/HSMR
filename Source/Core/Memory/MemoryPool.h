//
//  MemoryPool.h
//  Core
//
//  High-performance memory allocators for HSMR Engine
//  Reduces memory fragmentation and provides predictable allocation performance
//
#ifndef __HS_MEMORY_POOL_H__
#define __HS_MEMORY_POOL_H__

#include "Precompile.h"
#include "Core/Log.h"

#include <atomic>
#include <array>
#include <utility>
#include <concepts>

HS_NS_BEGIN

// Platform-specific aligned allocation utilities
namespace MemoryUtils
{
HS_CORE_API inline void* AlignedAlloc(size_t size, size_t alignment) noexcept
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
    void** header     = reinterpret_cast<void**>(aligned - sizeof(void*));
    *header           = ptr;
    return reinterpret_cast<void*>(aligned);
#endif
}

HS_CORE_API inline void AlignedFree(void* ptr) noexcept
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
} // namespace MemoryUtils

template <size_t BlockSize, size_t Alignment, uint8 BlockCount = 2 /* by default, use double buffering */>
    requires((Alignment > 0 && (Alignment & (Alignment - 1)) == 0) && BlockSize % Alignment == 0)
class LinearAllocator
{
public:
    LinearAllocator()
        : _offset{}
    {
        for (size_t i = 0; i < BlockCount; i++)
        {
            _block[i].reset(static_cast<uint8*>(MemoryUtils::AlignedAlloc(BlockSize, Alignment)));
            if (!_block[i])
            {
                HS_LOG(error, "Failed to allocate memory block %zu", i);
                throw std::bad_alloc();
            }
        }
    }

    ~LinearAllocator()
    {
        for (size_t i = 0; i < BlockCount; i++)
        {
            MemoryUtils::AlignedFree(_block[i].release());
        }
    }

    void* Allocate(size_t allocSize, uint8 allocIndex)
    {
        if (allocSize == 0 || allocSize > BlockSize)
        {
            HS_LOG(error, "Invalid allocation size: %zu", allocSize);
            return nullptr;
        }

        if (_offset[allocIndex] + allocSize > BlockSize)
        {
            HS_LOG(warning, "Current block exhausted. Consider implementing block switching or fallback allocator.");
            return nullptr; // For simplicity, we don't handle multiple blocks here
        }
        void* ptr = _block[allocIndex].get() + _offset[allocIndex];
        _offset[allocIndex] += (allocSize + Alignment - 1) & ~(Alignment - 1); // Align offset

        return ptr;
    }

    template <typename T, size_t AllocSize = sizeof(T), typename... Args>
    T* Allocate(uint8 allocIndex, Args&&... args)
    {
        void* mem = Allocate(AllocSize, allocIndex);
        T* ptr    = new (mem) T(std::forward<Args>(args)...);

        return ptr;
    }

    void Reset(uint8 allocIndex)
    {
        _offset[allocIndex] = 0;
    }

private:
    Scoped<uint8> _block[BlockCount];
    size_t _offset[BlockCount]{};
};

// TODO: Stack Allocator 구현하기 (필요할때)
//template <size_t BlockSize, uint8 BlockCount = 2>
//class StackAllocator
//{
//public:
//private:
//};

HS_NS_END

#endif // __HS_CORE_MEMORY_POOL_H__