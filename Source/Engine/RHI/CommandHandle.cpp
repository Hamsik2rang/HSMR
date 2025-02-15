#include "Engine/RHI/CommandHandle.h"

HS_NS_BEGIN

Semaphore::Semaphore()
    : RHIHandle(EType::SEMAPHORE)
{}

Semaphore::~Semaphore()
{}

Fence::Fence()
    : RHIHandle(EType::FENCE)
{}

Fence::~Fence()
{}

ResourceBarrier::ResourceBarrier()
    : RHIHandle(EType::RESOURCE_BARRIER)
{}

ResourceBarrier::~ResourceBarrier()
{}

CommandQueue::CommandQueue()
    : RHIHandle(EType::COMMAND_QUEUE)
{}

CommandQueue::~CommandQueue()
{}

CommandPool::CommandPool()
    : RHIHandle(EType::COMMAND_POOL)
{}

CommandPool::~CommandPool()
{}

CommandBuffer::CommandBuffer()
    : RHIHandle(EType::COMMAND_BUFFER)
{}

CommandBuffer::~CommandBuffer()
{}

HS_NS_END
