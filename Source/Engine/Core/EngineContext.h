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

HS_NS_BEGIN

class Renderer;

struct EngineContext
{
    //...
    
    const char* executablePath;
    const char* executableDirectory;
    const char* resourcePath;
};

EngineContext* hs_engine_create_context();
EngineContext* hs_engine_get_context();
EngineContext* hs_engine_destroy_context();

HS_NS_END

#endif /* EngineContext_hpp */
