//
//  MemoryPool.cpp
//  Core
//
//  Implementation of high-performance memory allocators
//
#include "Core/Memory/MemoryPool.h"
#include "Core/Log.h"

HS_NS_BEGIN

// Static member definitions
std::unique_ptr<LinearAllocator> MemoryManager::s_globalPool = nullptr;
MemoryStats MemoryManager::s_stats = {};

// Thread-local storage definitions  
thread_local std::unique_ptr<LinearAllocator> FrameAllocator::t_currentFrame = nullptr;
thread_local std::unique_ptr<LinearAllocator> FrameAllocator::t_previousFrame = nullptr;

// DefaultAllocator implementation
LinearAllocator& DefaultAllocator::GetGlobalPool()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        if (!MemoryManager::Initialize())
        {
            HS_LOG(crash, "Failed to initialize global memory pool");
        }
    });
    
    LinearAllocator* pool = MemoryManager::GetGlobalPool();
    if (!pool)
    {
        HS_LOG(crash, "Global memory pool not initialized");
    }
    
    return *pool;
}

HS_NS_END