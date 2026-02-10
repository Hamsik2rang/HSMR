//
//  Profiler.cpp
//  HSMR
//
//  Tracy profiler integration
//  Legacy implementation removed - now using Tracy
//

#include "Core/Profiler/Profiler.h"

// This file is kept for build system compatibility.
// All profiling is now handled via Tracy macros in ProfilerMacros.h
//
// The old ImPlot-based profiler has been replaced with Tracy,
// which provides superior profiling capabilities:
//   - Real-time frame profiler
//   - CPU zone timing with call stacks
//   - GPU timing (requires user implementation in VulkanDevice)
//   - Memory allocation tracking
//   - Lock contention analysis
//   - Remote profiling via Tracy server GUI
//
// To use Tracy:
// 1. Run Tracy server (tracy-profiler.exe from GitHub releases)
// 2. Launch your application with TRACY_ENABLE defined
// 3. Connect Tracy server to your application
//
// See TRACY_GPU_INTEGRATION.md for GPU profiling setup.
