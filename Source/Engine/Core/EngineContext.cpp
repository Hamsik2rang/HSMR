//
//  EngineContext.cpp
//  Engine
//
//  Created by Yongsik Im on 2/6/25.
//
#include "Engine/Core/EngineContext.h"

#include "Engine/Core/Log.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

EngineContext* s_engineContext;

EngineContext* hs_engine_initialize_context()
{
    if (nullptr != s_engineContext)
    {
        return s_engineContext;
    }
    
    s_engineContext = new EngineContext();
    s_engineContext->executablePath = SDL_GetBasePath();
    
}

EngineContext* hs_engine_get_context()
{
    if (nullptr != s_engineContext)
    {
        return s_engineContext;
    }

    HS_LOG(crash, "Engine Context isn't initialized.");
}

EngineContext* hs_engine_finalize_context()
{
    delete s_engineContext;
}

HS_NS_END

