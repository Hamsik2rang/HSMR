//
//  EngineContext.h
//  Engine
//
//  Created by Yongsik Im on 2025.2.6
//

#ifndef __HS_ENGINE_CONTEXT_H__
#define __HS_ENGINE_CONTEXT_H__

#include "Precompile.h"

#include "Engine/Core/Log.h"
#include "Engine/RHI/RHIContext.h"

#include <string>

HS_NS_BEGIN

class Application;
class Renderer;

enum class ERHIPlatform
{
    NONE   = 0,
    METAL  = 1,
    VULKAN = 2,
    //...
};

struct EngineContext
{
    const char* name = "";
    //...
    ERHIPlatform rhiPlatform;
    RHIContext*  rhiContext;

    std::string executablePath;
    std::string executableDirectory;
    std::string resourceDirectory;

    Application* app;
};

EngineContext* hs_engine_create_context(const char* name, ERHIPlatform rhiPlatform);
void           hs_engine_destroy_context();

EngineContext* hs_engine_get_context();
void           hs_engine_set_context(EngineContext* context);
RHIContext*    hs_engine_get_rhi_context();
void           hs_engine_set_rhi_context(void* rhiContext);

void hs_engine_set_position(uint32 x, uint32 y);
void hs_engine_get_position(uint32& outX, uint32& outY);

HS_NS_END

#endif /* EngineContext_hpp */
