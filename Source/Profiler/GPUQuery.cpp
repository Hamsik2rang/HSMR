//
//  GPUQuery.cpp
//  HSMR
//
//  GPU query wrapper for timing and statistics
//
#include "GPUQuery.h"
#include "Core/Log.h"

// Use HS_LOG macro instead of Log:: methods

HS_NS_BEGIN

// GPUQueryPool implementation
// Note: Actual GPU query implementation requires RHI support
// This is a placeholder that can be extended when RHI query support is added

GPUQueryPool::GPUQueryPool(EGPUQueryType type, uint32 queryCount)
    : _type(type)
    , _queryCount(queryCount)
{
    _results.resize(queryCount, 0);

    // TODO: Create platform-specific query pool
    // Vulkan: vkCreateQueryPool
    // Metal: MTLCounterSampleBuffer (requires macOS 10.15+)
}

GPUQueryPool::~GPUQueryPool()
{
    // TODO: Destroy platform-specific query pool
}

uint32 GPUQueryPool::AllocateQuery()
{
    if (_allocatedCount >= _queryCount)
    {
        HS_LOG(warning, "GPUQueryPool: No more queries available");
        return UINT32_MAX;
    }
    return _allocatedCount++;
}

void GPUQueryPool::ResetQueries()
{
    _allocatedCount = 0;
    _resultsValid = false;

    // TODO: Reset platform-specific queries
}

bool GPUQueryPool::GetTimestampResult(uint32 queryIndex, uint64& outTimestamp)
{
    if (queryIndex >= _queryCount) return false;

    // TODO: Fetch results from GPU
    // Vulkan: vkGetQueryPoolResults
    // Metal: resolveCounters

    // Placeholder: return cached result
    outTimestamp = _results[queryIndex];
    return _resultsValid;
}

bool GPUQueryPool::GetOcclusionResult(uint32 queryIndex, uint64& outSampleCount)
{
    if (queryIndex >= _queryCount) return false;

    outSampleCount = _results[queryIndex];
    return _resultsValid;
}

// GPUTimestampQuery implementation
GPUTimestampQuery::GPUTimestampQuery()
{
    // Create a small query pool for this timer
    _queryPool = new GPUQueryPool(EGPUQueryType::TIMESTAMP, 2);
}

GPUTimestampQuery::~GPUTimestampQuery()
{
    delete _queryPool;
}

void GPUTimestampQuery::RecordBegin(RHICommandBuffer* commandBuffer)
{
    if (!_queryPool || !commandBuffer) return;

    _queryPool->ResetQueries();
    _beginQueryIndex = _queryPool->AllocateQuery();

    // TODO: Record timestamp command
    // Vulkan: vkCmdWriteTimestamp
    // Metal: sampleCounters

    _resultAvailable = false;
}

void GPUTimestampQuery::RecordEnd(RHICommandBuffer* commandBuffer)
{
    if (!_queryPool || !commandBuffer) return;

    _endQueryIndex = _queryPool->AllocateQuery();

    // TODO: Record timestamp command
}

bool GPUTimestampQuery::GetElapsedTime(float& outMilliseconds)
{
    if (!_queryPool) return false;

    uint64 beginTimestamp = 0;
    uint64 endTimestamp = 0;

    if (!_queryPool->GetTimestampResult(_beginQueryIndex, beginTimestamp) ||
        !_queryPool->GetTimestampResult(_endQueryIndex, endTimestamp))
    {
        return false;
    }

    // Calculate elapsed time
    uint64 elapsed = endTimestamp - beginTimestamp;
    outMilliseconds = static_cast<float>(elapsed) * _timestampPeriod / 1000000.0f; // Convert ns to ms

    _resultAvailable = true;
    return true;
}

// GPUProfileScope implementation
GPUProfileScope::GPUProfileScope(RHICommandBuffer* commandBuffer, const char* name)
    : _commandBuffer(commandBuffer)
    , _name(name)
{
    _query.RecordBegin(commandBuffer);
}

GPUProfileScope::~GPUProfileScope()
{
    _query.RecordEnd(_commandBuffer);

    // Note: Results won't be available until GPU finishes
    // The profiler should collect these results in a later frame
}

HS_NS_END
