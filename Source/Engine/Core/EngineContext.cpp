#include "Engine/Core/EngineContext.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/FileSystem.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

namespace
{
EngineContext* s_engineContext;
};

EngineContext* hs_engine_initialize_context()
{
    if (nullptr != s_engineContext)
    {
        return s_engineContext;
    }

    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: %s\n", SDL_GetError());
        return nullptr;
    }

    // Inform SDL that we will be using metal for rendering. Without this hint initialization of metal renderer may fail.
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    // Enable native IME.
    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "1");

    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL+Metal example", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (window == nullptr)
    {
        printf("Error creating window: %s\n", SDL_GetError());
        return nullptr;
    }

    s_engineContext = new EngineContext();
    
    return s_engineContext;
}

EngineContext* hs_engine_get_context()
{
    if (nullptr != s_engineContext)
    {
        return s_engineContext;
    }

    HS_LOG(error, "Engine Context isn't initialized.");

    return nullptr;
}

EngineContext* hs_engine_finalize_context()
{
    SDL_Quit();

    delete s_engineContext;
}

HS_NS_END
