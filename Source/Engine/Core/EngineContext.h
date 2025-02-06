//
//  EngineContext.hpp
//  Engine
//
//  Created by Yongsik Im on 2/6/25.
//

#ifndef __HS_ENGINE_CONTEXT_H__
#define __HS_ENGINE_CONTEXT_H__

#include "Precompile.h"

#include <SDL3/SDL.h>

struct EngineContext
{
    
    struct SDLHandle
    {
        void* window; // SDL_Window*
        void* view;   // SDL_MetalView
        void* layer   // CAMetalLayer*
    }sdlHandle;
    const char* executablePath;
    const char* executableDirectory;
    const char* resourcePath;
};

EngineContext* hs_engine_create_context();
EngineContext* hs_engine_get_context();
EngineContext* hs_engine_destroy_context();

#endif /* EngineContext_hpp */
