//
//  ProfilerMacros.h
//  HSMR
//
//  Tracy Profiler wrapper macros
//

#pragma once

#include "Precompile.h"

#if defined(TRACY_ENABLE)

#include <tracy/Tracy.hpp>  // Located at Dependency/include/tracy/tracy/Tracy.hpp

// ===== CPU Profiling =====

// Frame boundary marker - call once per frame in main loop
#define HS_PROFILE_FRAME_MARK           FrameMark

// Named frame marker (for multiple frame types, e.g., "Render", "Physics")
#define HS_PROFILE_FRAME_MARK_N(name)   FrameMarkNamed(name)

// Scoped zone - automatically measures until end of scope
#define HS_PROFILE_ZONE()               ZoneScoped

// Named scoped zone
#define HS_PROFILE_ZONE_N(name)         ZoneScopedN(name)

// Named scoped zone with color (use 0xRRGGBB format)
#define HS_PROFILE_ZONE_NC(name, color) ZoneScopedNC(name, color)

// Function profiling (uses function name as zone name)
#define HS_PROFILE_FUNCTION()           ZoneScoped

// Add text to current zone
#define HS_PROFILE_ZONE_TEXT(text, len) ZoneText(text, len)

// Add value to current zone
#define HS_PROFILE_ZONE_VALUE(value)    ZoneValue(value)


// ===== Memory Profiling =====

// Track memory allocation
#define HS_PROFILE_ALLOC(ptr, size)     TracyAlloc(ptr, size)

// Track memory deallocation
#define HS_PROFILE_FREE(ptr)            TracyFree(ptr)

// Named memory allocation (for custom allocators)
#define HS_PROFILE_ALLOC_N(ptr, size, name)  TracyAllocN(ptr, size, name)
#define HS_PROFILE_FREE_N(ptr, name)         TracyFreeN(ptr, name)


// ===== Value Plotting =====

// Plot a value over time (appears as graph in Tracy)
#define HS_PROFILE_PLOT(name, value)    TracyPlot(name, value)

// Configure plot parameters
#define HS_PROFILE_PLOT_CONFIG(name, type, step, fill, color) \
    TracyPlotConfig(name, type, step, fill, color)


// ===== Messages/Logs =====

// Send a message to Tracy timeline
#define HS_PROFILE_MESSAGE(text, len)   TracyMessage(text, len)

// Colored message
#define HS_PROFILE_MESSAGE_C(text, len, color) TracyMessageC(text, len, color)

// Literal string message (compile-time length)
#define HS_PROFILE_MESSAGE_L(text)      TracyMessageL(text)


// ===== Lockable Synchronization =====

// Use these instead of std::mutex for lock profiling
#define HS_PROFILE_LOCKABLE(type, name) TracyLockable(type, name)
#define HS_PROFILE_SHARED_LOCKABLE(type, name) TracySharedLockable(type, name)


// ===== GPU Profiling (User Implementation Required) =====
// These macros require TracyVulkan.hpp and proper VkContext setup
// See TRACY_GPU_INTEGRATION.md for implementation guide
//
// IMPORTANT: Include TracyVulkan.hpp AFTER Vulkan headers in your RHI code:
//   #include <vulkan/vulkan.h>
//   #include <tracy/TracyVulkan.hpp>
//
// The GPU profiling macros are defined as no-ops here.
// They will be properly defined in RHI code after including TracyVulkan.hpp.

#define HS_PROFILE_GPU_CONTEXT(...)         ((void)0)
#define HS_PROFILE_GPU_CONTEXT_N(...)       ((void)0)
#define HS_PROFILE_GPU_CONTEXT_DESTROY(...) ((void)0)
#define HS_PROFILE_GPU_ZONE(...)            ((void)0)
#define HS_PROFILE_GPU_COLLECT(...)         ((void)0)


#else // TRACY_ENABLE not defined

// ===== No-op definitions when Tracy is disabled =====

#define HS_PROFILE_FRAME_MARK
#define HS_PROFILE_FRAME_MARK_N(name)
#define HS_PROFILE_ZONE()
#define HS_PROFILE_ZONE_N(name)
#define HS_PROFILE_ZONE_NC(name, color)
#define HS_PROFILE_FUNCTION()
#define HS_PROFILE_ZONE_TEXT(text, len)
#define HS_PROFILE_ZONE_VALUE(value)

#define HS_PROFILE_ALLOC(ptr, size)
#define HS_PROFILE_FREE(ptr)
#define HS_PROFILE_ALLOC_N(ptr, size, name)
#define HS_PROFILE_FREE_N(ptr, name)

#define HS_PROFILE_PLOT(name, value)
#define HS_PROFILE_PLOT_CONFIG(name, type, step, fill, color)

#define HS_PROFILE_MESSAGE(text, len)
#define HS_PROFILE_MESSAGE_C(text, len, color)
#define HS_PROFILE_MESSAGE_L(text)

#define HS_PROFILE_LOCKABLE(type, name)         type name
#define HS_PROFILE_SHARED_LOCKABLE(type, name)  type name

#define HS_PROFILE_GPU_CONTEXT(...)         ((void)0)
#define HS_PROFILE_GPU_CONTEXT_N(...)       ((void)0)
#define HS_PROFILE_GPU_CONTEXT_DESTROY(...) ((void)0)
#define HS_PROFILE_GPU_ZONE(...)            ((void)0)
#define HS_PROFILE_GPU_COLLECT(...)         ((void)0)

#endif // TRACY_ENABLE

// ===== Color Constants for Zones =====
namespace HS::Profile
{
    // Predefined colors for zone categorization
    constexpr uint32 ColorRender    = 0xE91E63;  // Pink - Rendering
    constexpr uint32 ColorPhysics   = 0x4CAF50;  // Green - Physics
    constexpr uint32 ColorAI        = 0xFF9800;  // Orange - AI/Logic
    constexpr uint32 ColorAudio     = 0x9C27B0;  // Purple - Audio
    constexpr uint32 ColorNetwork   = 0x00BCD4;  // Cyan - Network
    constexpr uint32 ColorIO        = 0x795548;  // Brown - File I/O
    constexpr uint32 ColorMemory    = 0xF44336;  // Red - Memory ops
    constexpr uint32 ColorUI        = 0x2196F3;  // Blue - UI
    constexpr uint32 ColorScene     = 0xFFEB3B;  // Yellow - Scene management
}
