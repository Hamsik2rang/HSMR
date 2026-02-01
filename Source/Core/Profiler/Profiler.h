//
//  Profiler.h
//  HSMR
//
//  CPU/GPU profiling with ImPlot visualization
//

//#ifndef __HS_PROFILER_H__
//#define __HS_PROFILER_H__
//
//#include "Precompile.h"
//
//#include <string>
//#include <deque>
//#include <unordered_map>
//#include <chrono>
//
//HS_NS_BEGIN
//
//// Timing data structure for profiler entries
//struct HS_PROFILER_API TimingData
//{
//    std::string name;
//    std::deque<float> history;  // History of timings (last N frames)
//    float current = 0.0f;
//    float average = 0.0f;
//    float min = 3.40282347E+38;
//    float max = 0.0f;
//    float total = 0.0f;
//
//    // For averaging calculation
//    int sampleCount = 0;
//
//    void AddSample(float ms);
//    void Reset();
//};
//
//// Scoped CPU timer helper
//class HS_PROFILER_API ScopedCPUTimer
//{
//public:
//    ScopedCPUTimer(class Profiler* profiler, const char* name);
//    ~ScopedCPUTimer();
//
//private:
//    class Profiler* _profiler;
//    std::string _name;
//};
//
//// Scoped GPU timer helper (placeholder - requires RHI query support)
//class HS_PROFILER_API ScopedGPUTimer
//{
//public:
//    ScopedGPUTimer(class Profiler* profiler, const char* name);
//    ~ScopedGPUTimer();
//
//private:
//    class Profiler* _profiler;
//    std::string _name;
//};
//
//// Profiler class with ImPlot integration
//class HS_PROFILER_API Profiler
//{
//public:
//    static constexpr int MAX_HISTORY = 256;
//
//    Profiler();
//    ~Profiler();
//
//    // Frame markers
//    void BeginFrame();
//    void EndFrame();
//
//    // CPU timing
//    void BeginCPUTimer(const char* name);
//    void EndCPUTimer(const char* name);
//
//    // GPU timing (requires RHI query support)
//    void BeginGPUTimer(const char* name);
//    void EndGPUTimer(const char* name);
//
//    // ImGui/ImPlot rendering
//    void DrawUI();
//
//    // Data access
//    float GetFrameTime() const { return _frameTime; }
//    float GetAverageFrameTime() const { return _averageFrameTime; }
//    float GetFPS() const { return _fps; }
//
//    const TimingData* GetCPUTiming(const char* name) const;
//    const TimingData* GetGPUTiming(const char* name) const;
//
//    // Settings
//    void SetEnabled(bool enabled) { _enabled = enabled; }
//    bool IsEnabled() const { return _enabled; }
//
//    void SetHistorySize(int size);
//    void ClearHistory();
//
//private:
//    void updateFrameStats();
//    void drawFrameTimeGraph();
//    void drawTimingBars();
//    void drawTimingTable();
//
//    bool _enabled = true;
//
//    // Frame timing
//    float _frameTime = 0.0f;
//    float _averageFrameTime = 0.0f;
//    float _fps = 0.0f;
//    float _frameTimeHistory[MAX_HISTORY] = { 0.0f };
//    int _frameIndex = 0;
//    int _historySize = MAX_HISTORY;
//
//    // CPU timings
//    std::unordered_map<std::string, TimingData> _cpuTimings;
//    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> _cpuStartTimes;
//
//    // GPU timings (placeholder data - actual GPU queries require RHI support)
//    std::unordered_map<std::string, TimingData> _gpuTimings;
//    std::unordered_map<std::string, float> _gpuStartTimes;
//
//    // Frame timing
//    std::chrono::high_resolution_clock::time_point _frameStartTime;
//    std::chrono::high_resolution_clock::time_point _lastFrameEndTime;
//
//    // Averaging
//    float _frameTimeSum = 0.0f;
//    int _frameCount = 0;
//    int _avgFrameCount = 60; // Average over 60 frames
//};
//
//// Convenience macros
//#define HS_CPU_TIMER(profiler, name) ScopedCPUTimer _timer_##__LINE__(profiler, name)
//#define HS_GPU_TIMER(profiler, name) ScopedGPUTimer _timer_##__LINE__(profiler, name)
//
//HS_NS_END
//
//#endif // __HS_PROFILER_PROFILER_H__
