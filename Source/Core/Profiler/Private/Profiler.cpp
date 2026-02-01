//
//  Profiler.cpp
//  HSMR
//
//  CPU/GPU profiling with ImPlot visualization
//
//#include "Precompile.h"

//#include "Profiler.h"
//#include "Core/Log.h"

//HS_NS_BEGIN

// // TimingData implementation
// void TimingData::AddSample(float ms)
// {
//     current = ms;
//     total += ms;
//     sampleCount++;

//     average = total / static_cast<float>(sampleCount);
//     min     = std::min(min, ms);
//     max     = std::max(max, ms);

//     // Add to history
//     history.push_back(ms);
//     while (history.size() > 256) // MAX_HISTORY
//     {
//         history.pop_front();
//     }
// }

// void TimingData::Reset()
// {
//     current     = 0.0f;
//     average     = 0.0f;
//     min         = HS_FLT_MAX;
//     max         = 0.0f;
//     total       = 0.0f;
//     sampleCount = 0;
//     history.clear();
// }

// // ScopedCPUTimer implementation
// ScopedCPUTimer::ScopedCPUTimer(Profiler* profiler, const char* name)
//     : _profiler(profiler)
//     , _name(name)
// {
//     if (_profiler && _profiler->IsEnabled())
//     {
//         _profiler->BeginCPUTimer(name);
//     }
// }

// ScopedCPUTimer::~ScopedCPUTimer()
// {
//     if (_profiler && _profiler->IsEnabled())
//     {
//         _profiler->EndCPUTimer(_name.c_str());
//     }
// }

// // ScopedGPUTimer implementation
// ScopedGPUTimer::ScopedGPUTimer(Profiler* profiler, const char* name)
//     : _profiler(profiler)
//     , _name(name)
// {
//     if (_profiler && _profiler->IsEnabled())
//     {
//         _profiler->BeginGPUTimer(name);
//     }
// }

// ScopedGPUTimer::~ScopedGPUTimer()
// {
//     if (_profiler && _profiler->IsEnabled())
//     {
//         _profiler->EndGPUTimer(_name.c_str());
//     }
// }

// // Profiler implementation
// Profiler::Profiler()
// {
//     _lastFrameEndTime = std::chrono::high_resolution_clock::now();
// }

// Profiler::~Profiler()
// {
// }

// void Profiler::BeginFrame()
// {
//     if (!_enabled) return;

//     _frameStartTime = std::chrono::high_resolution_clock::now();

//     // Clear per-frame timings
//     for (auto& [name, timing] : _cpuTimings)
//     {
//         // Keep history, just reset current
//     }
// }

// void Profiler::EndFrame()
// {
//     if (!_enabled) return;

//     auto now = std::chrono::high_resolution_clock::now();

//     // Calculate frame time
//     auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
//         now - _frameStartTime
//     );
//     _frameTime = duration.count() / 1000.0f; // Convert to ms

//     // Update frame time history
//     _frameTimeHistory[_frameIndex] = _frameTime;
//     _frameIndex                    = (_frameIndex + 1) % _historySize;

//     // Update statistics
//     updateFrameStats();

//     _lastFrameEndTime = now;
// }

// void Profiler::BeginCPUTimer(const char* name)
// {
//     if (!_enabled) return;

//     _cpuStartTimes[name] = std::chrono::high_resolution_clock::now();
// }

// void Profiler::EndCPUTimer(const char* name)
// {
//     if (!_enabled) return;

//     auto it = _cpuStartTimes.find(name);
//     if (it == _cpuStartTimes.end()) return;

//     auto now      = std::chrono::high_resolution_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
//         now - it->second
//     );
//     float ms = duration.count() / 1000.0f;

//     // Initialize timing data if needed
//     auto& timing = _cpuTimings[name];
//     timing.name  = name;
//     timing.AddSample(ms);

//     _cpuStartTimes.erase(it);
// }

// void Profiler::BeginGPUTimer(const char* name)
// {
//     if (!_enabled) return;

//     // Placeholder - actual GPU timing requires RHI query support
//     // For now, we'll simulate with CPU timing
//     _gpuStartTimes[name] = static_cast<float>(
//         std::chrono::duration_cast<std::chrono::microseconds>(
//             std::chrono::high_resolution_clock::now().time_since_epoch()
//         )
//             .count() /
//         1000.0f
//     );
// }

// void Profiler::EndGPUTimer(const char* name)
// {
//     if (!_enabled) return;

//     auto it = _gpuStartTimes.find(name);
//     if (it == _gpuStartTimes.end()) return;

//     float now = static_cast<float>(
//         std::chrono::duration_cast<std::chrono::microseconds>(
//             std::chrono::high_resolution_clock::now().time_since_epoch()
//         )
//             .count() /
//         1000.0f
//     );

//     float ms = now - it->second;

//     // Initialize timing data if needed
//     auto& timing = _gpuTimings[name];
//     timing.name  = name;
//     timing.AddSample(ms);

//     _gpuStartTimes.erase(it);
// }

// void Profiler::DrawUI()
// {
//     if (!_enabled) return;

//     if (ImGui::Begin("Profiler"))
//     {
//         // Header stats
//         ImGui::Text("Frame Time: %.3f ms", _frameTime);
//         ImGui::SameLine();
//         ImGui::Text("Avg: %.3f ms", _averageFrameTime);
//         ImGui::SameLine();
//         ImGui::Text("FPS: %.1f", _fps);

//         ImGui::Separator();

//         // Frame time graph
//         drawFrameTimeGraph();

//         ImGui::Separator();

//         // Tabs for CPU and GPU timings
//         if (ImGui::BeginTabBar("ProfilerTabs"))
//         {
//             if (ImGui::BeginTabItem("CPU Timings"))
//             {
//                 drawTimingTable();
//                 ImGui::EndTabItem();
//             }

//             if (ImGui::BeginTabItem("GPU Timings"))
//             {
//                 // GPU timing table
//                 if (ImGui::BeginTable("GPUTimings", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
//                 {
//                     ImGui::TableSetupColumn("Name");
//                     ImGui::TableSetupColumn("Current");
//                     ImGui::TableSetupColumn("Avg");
//                     ImGui::TableSetupColumn("Min");
//                     ImGui::TableSetupColumn("Max");
//                     ImGui::TableHeadersRow();

//                     for (const auto& [name, data] : _gpuTimings)
//                     {
//                         ImGui::TableNextRow();
//                         ImGui::TableNextColumn();
//                         ImGui::Text("%s", name.c_str());
//                         ImGui::TableNextColumn();
//                         ImGui::Text("%.3f ms", data.current);
//                         ImGui::TableNextColumn();
//                         ImGui::Text("%.3f ms", data.average);
//                         ImGui::TableNextColumn();
//                         ImGui::Text("%.3f ms", data.min == FLT_MAX ? 0.0f : data.min);
//                         ImGui::TableNextColumn();
//                         ImGui::Text("%.3f ms", data.max);
//                     }

//                     ImGui::EndTable();
//                 }
//                 ImGui::EndTabItem();
//             }

//             if (ImGui::BeginTabItem("History"))
//             {
//                 drawTimingBars();
//                 ImGui::EndTabItem();
//             }

//             ImGui::EndTabBar();
//         }

//         // Controls
//         ImGui::Separator();
//         if (ImGui::Button("Clear History"))
//         {
//             ClearHistory();
//         }
//         ImGui::SameLine();
//         ImGui::Checkbox("Enabled", &_enabled);
//     }
//     ImGui::End();
// }

// const TimingData* Profiler::GetCPUTiming(const char* name) const
// {
//     auto it = _cpuTimings.find(name);
//     if (it != _cpuTimings.end())
//     {
//         return &it->second;
//     }
//     return nullptr;
// }

// const TimingData* Profiler::GetGPUTiming(const char* name) const
// {
//     auto it = _gpuTimings.find(name);
//     if (it != _gpuTimings.end())
//     {
//         return &it->second;
//     }
//     return nullptr;
// }

// void Profiler::SetHistorySize(int size)
// {
//     _historySize = std::min(size, MAX_HISTORY);
// }

// void Profiler::ClearHistory()
// {
//     for (int i = 0; i < MAX_HISTORY; ++i)
//     {
//         _frameTimeHistory[i] = 0.0f;
//     }
//     _frameIndex = 0;

//     for (auto& [name, timing] : _cpuTimings)
//     {
//         timing.Reset();
//     }

//     for (auto& [name, timing] : _gpuTimings)
//     {
//         timing.Reset();
//     }

//     _frameTimeSum = 0.0f;
//     _frameCount   = 0;
// }

// void Profiler::updateFrameStats()
// {
//     // Update running average
//     _frameTimeSum += _frameTime;
//     _frameCount++;

//     if (_frameCount >= _avgFrameCount)
//     {
//         _averageFrameTime = _frameTimeSum / static_cast<float>(_frameCount);
//         _fps              = 1000.0f / _averageFrameTime;
//         _frameTimeSum     = 0.0f;
//         _frameCount       = 0;
//     }
// }

// void Profiler::drawFrameTimeGraph()
// {
// #if HAS_IMPLOT
//     // Use ImPlot for better graphs
//     if (ImPlot::BeginPlot("Frame Time", ImVec2(-1, 150)))
//     {
//         ImPlot::SetupAxes("Frame", "ms", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
//         ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 33.3, ImPlotCond_Once); // 30fps target

//         // Reorder history for continuous plot
//         float orderedHistory[MAX_HISTORY];
//         for (int i = 0; i < _historySize; ++i)
//         {
//             int idx           = (_frameIndex + i) % _historySize;
//             orderedHistory[i] = _frameTimeHistory[idx];
//         }

//         ImPlot::PlotLine("Frame Time", orderedHistory, _historySize);

//         // Draw target frame time lines
//         float target60fps = 16.67f;
//         float target30fps = 33.33f;
//         ImPlot::PlotInfLines("60 FPS", &target60fps, 1, ImPlotInfLinesFlags_Horizontal);
//         ImPlot::PlotInfLines("30 FPS", &target30fps, 1, ImPlotInfLinesFlags_Horizontal);

//         ImPlot::EndPlot();
//     }
// #else
//     // Fallback to ImGui plot
//     ImGui::PlotLines("Frame Time (ms)", _frameTimeHistory, _historySize, _frameIndex, nullptr, 0.0f, 33.3f, ImVec2(0, 80));

//     ImGui::Text("Target 60 FPS: 16.67ms | Target 30 FPS: 33.33ms");
// #endif
// }

// void Profiler::drawTimingBars()
// {
// #if HAS_IMPLOT
//     // Bar chart for CPU timings
//     if (!_cpuTimings.empty())
//     {
//         ImGui::Text("CPU Timings");

//         if (ImPlot::BeginPlot("CPU Timings", ImVec2(-1, 200)))
//         {
//             ImPlot::SetupAxes("Timer", "ms");

//             std::vector<const char*> labels;
//             std::vector<float> values;

//             for (const auto& [name, data] : _cpuTimings)
//             {
//                 labels.push_back(name.c_str());
//                 values.push_back(data.current);
//             }

//             ImPlot::SetupAxisTicks(ImAxis_X1, 0, (double)labels.size() - 1, (int)labels.size(), labels.data(), false);
//             ImPlot::PlotBars("Current", values.data(), (int)values.size(), 0.67);

//             ImPlot::EndPlot();
//         }
//     }

//     // Bar chart for GPU timings
//     if (!_gpuTimings.empty())
//     {
//         ImGui::Text("GPU Timings");

//         if (ImPlot::BeginPlot("GPU Timings", ImVec2(-1, 200)))
//         {
//             ImPlot::SetupAxes("Timer", "ms");

//             std::vector<const char*> labels;
//             std::vector<float> values;

//             for (const auto& [name, data] : _gpuTimings)
//             {
//                 labels.push_back(name.c_str());
//                 values.push_back(data.current);
//             }

//             ImPlot::SetupAxisTicks(ImAxis_X1, 0, (double)labels.size() - 1, (int)labels.size(), labels.data(), false);
//             ImPlot::PlotBars("Current", values.data(), (int)values.size(), 0.67);

//             ImPlot::EndPlot();
//         }
//     }
// #else
//     // Fallback without ImPlot
//     ImGui::Text("ImPlot not available - showing raw data");

//     for (const auto& [name, data] : _cpuTimings)
//     {
//         float fraction = data.current / 16.67f; // Relative to 60fps target
//         ImGui::ProgressBar(fraction, ImVec2(-1, 0), name.c_str());
//     }
// #endif
// }

// void Profiler::drawTimingTable()
// {
//     if (ImGui::BeginTable("CPUTimings", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable))
//     {
//         ImGui::TableSetupColumn("Name");
//         ImGui::TableSetupColumn("Current");
//         ImGui::TableSetupColumn("Avg");
//         ImGui::TableSetupColumn("Min");
//         ImGui::TableSetupColumn("Max");
//         ImGui::TableHeadersRow();

//         for (const auto& [name, data] : _cpuTimings)
//         {
//             ImGui::TableNextRow();
//             ImGui::TableNextColumn();
//             ImGui::Text("%s", name.c_str());
//             ImGui::TableNextColumn();
//             ImGui::Text("%.3f ms", data.current);
//             ImGui::TableNextColumn();
//             ImGui::Text("%.3f ms", data.average);
//             ImGui::TableNextColumn();
//             ImGui::Text("%.3f ms", data.min == FLT_MAX ? 0.0f : data.min);
//             ImGui::TableNextColumn();
//             ImGui::Text("%.3f ms", data.max);
//         }

//         ImGui::EndTable();
//     }
// }

//HS_NS_END
