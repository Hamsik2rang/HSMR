//
//  ProfileDataCollector.cpp
//  Core
//
//  In-engine zone timing data collector implementation.
//

#include "Core/Profiler/ProfileDataCollector.h"
#include "Core/HAL/Timer.h"

#include <algorithm>
#include <cmath>

HS_NS_BEGIN

ProfileDataCollector& ProfileDataCollector::Get()
{
    static ProfileDataCollector s_instance;
    return s_instance;
}

void ProfileDataCollector::Finalize()
{
    ProfileDataCollector& collector = Get();
    collector._currentFrame.clear();
    collector._lastFrame.clear();
    collector._zoneStack.clear();
    collector._statsMap.clear();
    collector._currentDepth = 0;
}

void ProfileDataCollector::BeginFrame()
{
    // Swap: current becomes last, current is cleared for new frame
    _lastFrame.swap(_currentFrame);
    _currentFrame.clear();
    _zoneStack.clear();
    _currentDepth = 0;

    // Update per-zone statistics from last frame
    for (const auto& zone : _lastFrame)
    {
        auto& history = _statsMap[zone.name];
        history.samples[history.writeIndex] = zone.durationMs;
        history.writeIndex = (history.writeIndex + 1) % STATS_HISTORY;
        if (history.sampleCount < STATS_HISTORY)
        {
            history.sampleCount++;
        }
    }
}

void ProfileDataCollector::PushZone(const char* name, uint32 color)
{
    int recordIndex = static_cast<int>(_currentFrame.size());

    ZoneRecord record;
    record.name       = name;
    record.durationMs = 0.0f;
    record.depth      = _currentDepth;
    record.color      = color;
    _currentFrame.push_back(record);

    OpenZone open;
    open.recordIndex = recordIndex;
    open.startTimeMs = Timer::GetElapsedMilliseconds();
    _zoneStack.push_back(open);

    _currentDepth++;
}

void ProfileDataCollector::PopZone()
{
    if (_zoneStack.empty())
    {
        return;
    }

    OpenZone& open = _zoneStack.back();
    double endTimeMs = Timer::GetElapsedMilliseconds();
    float durationMs = static_cast<float>(endTimeMs - open.startTimeMs);

    if (open.recordIndex < static_cast<int>(_currentFrame.size()))
    {
        _currentFrame[open.recordIndex].durationMs = durationMs;
    }

    _zoneStack.pop_back();
    _currentDepth--;
}

ZoneStats ProfileDataCollector::GetZoneStats(const char* name) const
{
    ZoneStats stats{};
    auto it = _statsMap.find(name);
    if (it == _statsMap.end() || it->second.sampleCount == 0)
    {
        return stats;
    }

    const ZoneHistory& history = it->second;
    float sum = 0.0f;
    float minVal = 1e9f;
    float maxVal = 0.0f;

    for (int i = 0; i < history.sampleCount; ++i)
    {
        float v = history.samples[i];
        sum += v;
        minVal = std::min(minVal, v);
        maxVal = std::max(maxVal, v);
    }

    stats.avgMs = sum / static_cast<float>(history.sampleCount);
    stats.minMs = minVal;
    stats.maxMs = maxVal;
    return stats;
}

HS_NS_END
