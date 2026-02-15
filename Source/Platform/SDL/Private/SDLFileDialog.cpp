//
//  SDLFileDialog.cpp
//  Platform
//
//  Created by Claude on 2/12/26.
//

#include "Platform/SDL/SDLFileDialog.h"

#include <atomic>

#include <SDL3/SDL.h>

#ifdef __APPLE__
struct HSFileDialogFilter
{
    const char* name;
    const char* pattern;
};
extern std::string HSOpenFileDialog_Native(const HSFileDialogFilter* filters, int filterCount,
                                           const char* defaultLocation);
extern std::string HSSaveFileDialog_Native(const HSFileDialogFilter* filters, int filterCount,
                                           const char* defaultLocation);
extern std::string HSOpenFolderDialog_Native(const char* defaultLocation);
#endif

HS_NS_BEGIN

#ifndef __APPLE__
struct FileDialogContext
{
    std::string result;
    std::atomic<bool> done{false};
};

static void FileDialogCallback(void* userdata, const char* const* filelist, int filter)
{
    auto* ctx = static_cast<FileDialogContext*>(userdata);
    if (filelist && filelist[0])
    {
        ctx->result = filelist[0];
    }
    ctx->done.store(true);
}

static std::string WaitForDialog(FileDialogContext& ctx)
{
    while (!ctx.done.load())
    {
        SDL_PumpEvents();
        SDL_Delay(10);
    }
    return ctx.result;
}
#endif

std::string FileDialog::OpenFile(const FileDialogFilter* filters, uint32 filterCount,
                                 const char* defaultLocation)
{
#ifdef __APPLE__
    // Use native NSOpenPanel — SDL's async approach deadlocks during ImGui frames
    return HSOpenFileDialog_Native(reinterpret_cast<const HSFileDialogFilter*>(filters),
                                   static_cast<int>(filterCount), defaultLocation);
#else
    FileDialogContext ctx;

    std::vector<SDL_DialogFileFilter> sdlFilters;
    if (filters && filterCount > 0)
    {
        sdlFilters.resize(filterCount);
        for (uint32 i = 0; i < filterCount; ++i)
        {
            sdlFilters[i].name    = filters[i].name;
            sdlFilters[i].pattern = filters[i].pattern;
        }
    }

    SDL_ShowOpenFileDialog(
        FileDialogCallback, &ctx, nullptr,
        sdlFilters.empty() ? nullptr : sdlFilters.data(),
        static_cast<int>(sdlFilters.size()),
        defaultLocation, false);

    return WaitForDialog(ctx);
#endif
}

std::string FileDialog::SaveFile(const FileDialogFilter* filters, uint32 filterCount,
                                 const char* defaultLocation)
{
#ifdef __APPLE__
    return HSSaveFileDialog_Native(reinterpret_cast<const HSFileDialogFilter*>(filters),
                                    static_cast<int>(filterCount), defaultLocation);
#else
    FileDialogContext ctx;

    std::vector<SDL_DialogFileFilter> sdlFilters;
    if (filters && filterCount > 0)
    {
        sdlFilters.resize(filterCount);
        for (uint32 i = 0; i < filterCount; ++i)
        {
            sdlFilters[i].name    = filters[i].name;
            sdlFilters[i].pattern = filters[i].pattern;
        }
    }

    SDL_ShowSaveFileDialog(
        FileDialogCallback, &ctx, nullptr,
        sdlFilters.empty() ? nullptr : sdlFilters.data(),
        static_cast<int>(sdlFilters.size()),
        defaultLocation);

    return WaitForDialog(ctx);
#endif
}

std::string FileDialog::OpenFolder(const char* defaultLocation)
{
#ifdef __APPLE__
    return HSOpenFolderDialog_Native(defaultLocation);
#else
    FileDialogContext ctx;

    SDL_ShowOpenFolderDialog(
        FileDialogCallback, &ctx, nullptr,
        defaultLocation, false);

    return WaitForDialog(ctx);
#endif
}

HS_NS_END
