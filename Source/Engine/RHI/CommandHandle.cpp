#include "Engine/RHI/CommandHandle.h"

HS_NS_BEGIN

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
