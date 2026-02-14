#include "RHI/CommandHandle.h"

HS_NS_BEGIN

RHICommandQueue::RHICommandQueue(const char* name)
	: RHIHandle(EType::CommandQueue, name)
{}

RHICommandQueue::~RHICommandQueue()
{}

RHICommandPool::RHICommandPool(const char* name)
	: RHIHandle(EType::CommandPool, name)
{}

RHICommandPool::~RHICommandPool()
{}

RHICommandBuffer::RHICommandBuffer(const char* name)
	: RHIHandle(EType::CommandBuffer, name)
{}

RHICommandBuffer::~RHICommandBuffer()
{}

HS_NS_END
