#include "Platform/Win/WinSystemContext.h"

#include "Core/SystemContext.h"
#include "Core/HAL/FileSystem.h"

#include "Core/Log.h"

HS_NS_BEGIN

#ifdef __SDL__
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
static bool s_sdlInitialized = false;
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

bool SystemContext::initializePlatform()
{
#ifdef __SDL__

    // Initialize SDL if not already done
    if (!s_sdlInitialized)
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            HS_LOG(crash, "Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }
        s_sdlInitialized = true;
    }

    executableDirectory = std::string(SDL_GetBasePath());
    executablePath      = executableDirectory + "HSMR.exe";
    assetDirectory      = executableDirectory + "Assets" + HS_DIR_SEPERATOR;
#else
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    char path[MAX_PATH] = {0};
    DWORD length        = GetModuleFileNameA(nullptr, path, MAX_PATH);

    if (length == 0 || length == MAX_PATH)
    {
        // Try with longer buffer for long paths
        WCHAR longPath[HS_CHAR_INIT_LONG_LENGTH] = {0};
        DWORD longLength                         = GetModuleFileNameW(nullptr, longPath, HS_CHAR_INIT_LONG_LENGTH);

        if (longLength > 0 && longLength < 32768)
        {
            executablePath = FileSystem::Utf16ToUtf8(std::wstring(longPath));
        }
    }
    else
    {
        executablePath = std::string(path);
    }

    executableDirectory = FileSystem::GetDirectory(executablePath);
    assetDirectory      = executableDirectory + "Assets" + HS_DIR_SEPERATOR;
#endif

    return true;
}

void SystemContext::finalizePlatform()
{
    //...
}

HS_NS_END
