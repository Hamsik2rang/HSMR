#include "Engine/Core/EngineContext.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/FileSystem.h"

#include "Engine/RHI/Metal/RHIContextMetal.h"

#include <SDL3/SDL.h>
#include <string>

HS_NS_BEGIN

static EngineContext* s_engineContext;

EngineContext* hs_engine_create_context(const std::string& name, ERHIPlatform rhiPlatform)
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

    s_engineContext                      = new EngineContext();
    s_engineContext->name                = name;
    s_engineContext->rhiPlatform         = rhiPlatform;
    s_engineContext->executableDirectory = SDL_GetBasePath();
    s_engineContext->executablePath      = s_engineContext->executableDirectory + s_engineContext->name;
    s_engineContext->resourceDirectory   = s_engineContext->executableDirectory + std::string("Resource/");

    switch (rhiPlatform)
    {
        case ERHIPlatform::METAL:
        {
            s_engineContext->rhiContext = new RHIContextMetal();
        }
        break;
        default:
            break;
    }

    return s_engineContext;
}

EngineContext* hs_engine_get_context()
{
    if (nullptr != s_engineContext)
    {
        return s_engineContext;
    }

    HS_LOG(crash, "Engine Context isn't initialized.");

    return nullptr;
}

void hs_engine_set_context(EngineContext* context)
{
    s_engineContext = context;
}

RHIContext* hs_engine_get_rhi_context()
{
    return hs_engine_get_context()->rhiContext;
}

void hs_engine_set_rhi_context(RHIContext* rhiContext)
{
    hs_engine_get_context()->rhiContext = rhiContext;
}

EngineContext* hs_engine_destroy_context()
{
    SDL_Quit();

    delete s_engineContext;
}

HS_NS_END
