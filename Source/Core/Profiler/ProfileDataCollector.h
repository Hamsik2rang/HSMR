//
//  ProfileDataCollector.h
//  Core
//
//  In-engine zone timing data collector for the editor profiler panel.
//

#ifndef __HS_CORE_PROFILE_DATA_COLLECTOR_H__
#define __HS_CORE_PROFILE_DATA_COLLECTOR_H__

#include "Precompile.h"

#include <vector>
#include <string>
#include <unordered_map>

HS_NS_BEGIN

struct ZoneRecord
{
    const char* name    = nullptr;
    float durationMs    = 0.0f;
    int depth           = 0;
    uint32 color        = 0xFFFFFF;
};

struct ZoneStats
{
    float avgMs = 0.0f;
    float minMs = 0.0f;
    float maxMs = 0.0f;
};

class HS_CORE_API ProfileDataCollector
{
public:
    static ProfileDataCollector& Get();
    static void Finalize();

    void BeginFrame();
    void PushZone(const char* name, uint32 color);
    void PopZone();

    const std::vector<ZoneRecord>& GetLastFrameZones() const { return _lastFrame; }
    ZoneStats GetZoneStats(const char* name) const;

private:
    ProfileDataCollector() = default;
    ~ProfileDataCollector() = default;
    ProfileDataCollector(const ProfileDataCollector&) = delete;
    ProfileDataCollector& operator=(const ProfileDataCollector&) = delete;

    // Double buffered zone records
    std::vector<ZoneRecord> _currentFrame;
    std::vector<ZoneRecord> _lastFrame;

    // Stack for tracking open zones
    struct OpenZone
    {
        int recordIndex;
        double startTimeMs;
    };
    std::vector<OpenZone> _zoneStack;
    int _currentDepth = 0;

    // Per-zone statistics (rolling window)
    static constexpr int STATS_HISTORY = 120;

    struct ZoneHistory
    {
        float samples[STATS_HISTORY] = {};
        int writeIndex = 0;
        int sampleCount = 0;
    };
    std::unordered_map<std::string, ZoneHistory> _statsMap;
};

HS_NS_END

#endif /* __HS_CORE_PROFILE_DATA_COLLECTOR_H__ */
