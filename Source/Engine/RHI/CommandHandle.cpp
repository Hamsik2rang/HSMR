#include "Engine/RHI/CommandHandle.h"

HS_NS_BEGIN

Semaphore::Semaphore(const char* name)
	: RHIHandle(EType::SEMAPHORE, name)
{}

Semaphore::~Semaphore()
{}

Fence::Fence(const char* name)
	: RHIHandle(EType::FENCE, name)
{}

Fence::~Fence()
{}

ResourceBarrier::ResourceBarrier(const char* name)
	: RHIHandle(EType::RESOURCE_BARRIER, name)
{}

ResourceBarrier::~ResourceBarrier()
{}

CommandQueue::CommandQueue(const char* name)
	: RHIHandle(EType::COMMAND_QUEUE, name)
{}

CommandQueue::~CommandQueue()
{}

CommandPool::CommandPool(const char* name)
	: RHIHandle(EType::COMMAND_POOL, name)
{}

CommandPool::~CommandPool()
{}

CommandBuffer::CommandBuffer(const char* name)
	: RHIHandle(EType::COMMAND_BUFFER, name)
{}

CommandBuffer::~CommandBuffer()
{}

HS_NS_END
