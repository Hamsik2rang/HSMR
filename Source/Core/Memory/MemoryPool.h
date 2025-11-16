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

HS_NS_BEGIN

// Forward declarations
class LinearAllocator;
class StackAllocator;

// Platform-specific aligned allocation utilities
namespace MemoryUtils
{
    HS_API inline void* AlignedAlloc(size_t size, size_t alignment) noexcept
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

    HS_API inline void AlignedFree(void* ptr) noexcept
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

HS_NS_END

#endif // __HS_CORE_MEMORY_POOL_H__