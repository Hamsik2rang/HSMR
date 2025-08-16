#include "RHI/CommandHandle.h"

HS_NS_BEGIN

RHICommandQueue::RHICommandQueue(const char* name)
	: RHIHandle(EType::COMMAND_QUEUE, name)
{}

RHICommandQueue::~RHICommandQueue()
{}

RHICommandPool::RHICommandPool(const char* name)
	: RHIHandle(EType::COMMAND_POOL, name)
{}

RHICommandPool::~RHICommandPool()
{}

RHICommandBuffer::RHICommandBuffer(const char* name)
	: RHIHandle(EType::COMMAND_BUFFER, name)
{}

RHICommandBuffer::~RHICommandBuffer()
{}

HS_NS_END
