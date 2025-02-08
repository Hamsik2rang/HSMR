//
//  EngineContext.hpp
//  Engine
//
//  Created by Yongsik Im on 2/6/25.
//

#ifndef __HS_ENGINE_CONTEXT_H__
#define __HS_ENGINE_CONTEXT_H__

#include "Precompile.h"

#include "Engine/Core/Log.h"

#include <SDL3/SDL.h>

#include <string>

HS_NS_BEGIN

class Renderer;

enum class ERHIPlatform
{
    NONE  = 0,
    METAL = 1,
    //...
};

struct EngineContext
{
    std::string name = "";
    //...
    ERHIPlatform rhiPlatform;

    std::string executablePath;
    std::string executableDirectory;
    std::string resourceDirectory;
};

EngineContext* hs_engine_create_context(const std::string& name, ERHIPlatform rhiPlatform);
EngineContext* hs_engine_get_context();
void hs_engine_set_context(EngineContext* context);
EngineContext* hs_engine_destroy_context();
void hs_engine_set_position(uint32 x, uint32 y);
void hs_engine_get_position(uint32& outX, uint32& outY);
void hs_engine_set_size(uint32 width, uint32 height);
void hs_engine_get_size(uint32& outWidth, uint32& outHeight);
HS_NS_END

#endif /* EngineContext_hpp */
