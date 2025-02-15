//
//  CommandHandle.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//
#ifndef __HS_COMMAND_HANDLE_H__
#define __HS_COMMAND_HANDLE_H__

#include "Precompile.h"

#include "Engine/RHI/RHIDefinition.h"

HS_NS_BEGIN

struct Semaphore : public RHIHandle
{
    Semaphore();
    ~Semaphore() override;
};

struct Fence : public RHIHandle
{
    Fence();
    ~Fence() override;
};

struct ResourceBarrier : public RHIHandle
{
    ResourceBarrier();
    ~ResourceBarrier() override;
};

struct CommandQueue : public RHIHandle
{
    CommandQueue();
    ~CommandQueue() override;
};

struct CommandPool : public RHIHandle
{
    CommandPool();
    ~CommandPool() override;
};

struct CommandBuffer : public RHIHandle
{
    CommandBuffer();
    ~CommandBuffer() override;
};

HS_NS_END

#endif /* __HS_COMMAND_HANDLE_H__ */
