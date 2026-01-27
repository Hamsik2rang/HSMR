//
//  GPUQuery.h
//  HSMR
//
//  GPU query wrapper for timing and statistics
//
#ifndef __HS_PROFILER_GPU_QUERY_H__
#define __HS_PROFILER_GPU_QUERY_H__

#include "Precompile.h"
#include <vector>

// Forward declarations
struct RHICommandBuffer;

HS_NS_BEGIN

// GPU query types
enum class EGPUQueryType
{
    TIMESTAMP,
    OCCLUSION,
    PIPELINE_STATISTICS
};

// GPU query pool - manages multiple queries of the same type
class HS_PROFILER_API GPUQueryPool
{
public:
    GPUQueryPool(EGPUQueryType type, uint32 queryCount);
    ~GPUQueryPool();

    // Query management
    uint32 AllocateQuery();
    void ResetQueries();

    // Get results
    bool GetTimestampResult(uint32 queryIndex, uint64& outTimestamp);
    bool GetOcclusionResult(uint32 queryIndex, uint64& outSampleCount);

    // Properties
    EGPUQueryType GetType() const { return _type; }
    uint32 GetQueryCount() const { return _queryCount; }
    uint32 GetAllocatedCount() const { return _allocatedCount; }

    // Platform-specific handle
    void* GetNativeHandle() const { return _nativeHandle; }

private:
    EGPUQueryType _type;
    uint32 _queryCount;
    uint32 _allocatedCount = 0;
    void* _nativeHandle = nullptr;

    // Results buffer
    std::vector<uint64> _results;
    bool _resultsValid = false;
};

// GPU timestamp query helper
class HS_PROFILER_API GPUTimestampQuery
{
public:
    GPUTimestampQuery();
    ~GPUTimestampQuery();

    // Record timestamps
    void RecordBegin(RHICommandBuffer* commandBuffer);
    void RecordEnd(RHICommandBuffer* commandBuffer);

    // Get elapsed time in milliseconds
    // Returns false if results not yet available
    bool GetElapsedTime(float& outMilliseconds);

    // Check if results are available
    bool IsResultAvailable() const { return _resultAvailable; }

private:
    GPUQueryPool* _queryPool = nullptr;
    uint32 _beginQueryIndex = 0;
    uint32 _endQueryIndex = 0;
    bool _resultAvailable = false;
    float _timestampPeriod = 1.0f; // Nanoseconds per timestamp tick
};

// GPU profiler scope for automatic timing
class HS_PROFILER_API GPUProfileScope
{
public:
    GPUProfileScope(RHICommandBuffer* commandBuffer, const char* name);
    ~GPUProfileScope();

private:
    RHICommandBuffer* _commandBuffer;
    std::string _name;
    GPUTimestampQuery _query;
};

// Macro for easy GPU profiling
#define HS_GPU_PROFILE_SCOPE(cmdBuffer, name) GPUProfileScope _gpuScope##__LINE__(cmdBuffer, name)

HS_NS_END

#endif // __HS_PROFILER_GPU_QUERY_H__
