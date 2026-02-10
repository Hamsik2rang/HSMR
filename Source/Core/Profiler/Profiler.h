//
//  Profiler.h
//  HSMR
//
//  Unified profiling header - includes Tracy macros
//

#pragma once

// Include Tracy wrapper macros
#include "Core/Profiler/ProfilerMacros.h"

// Legacy profiler implementation has been replaced by Tracy.
// Use the HS_PROFILE_* macros defined in ProfilerMacros.h
//
// Quick Reference:
//   HS_PROFILE_FRAME_MARK          - Call once per frame in main loop
//   HS_PROFILE_FUNCTION()          - Profile entire function
//   HS_PROFILE_ZONE_N("name")      - Named profiling zone
//   HS_PROFILE_ZONE_NC("name", color) - Colored zone
//
// GPU Profiling (requires user implementation):
//   See TRACY_GPU_INTEGRATION.md for VulkanDevice setup
//
// Memory Profiling:
//   HS_PROFILE_ALLOC(ptr, size)    - Track allocation
//   HS_PROFILE_FREE(ptr)           - Track deallocation
