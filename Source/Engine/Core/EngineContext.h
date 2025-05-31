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
    ERHIPlatform rhiPlatform = ERHIPlatform::NONE;
    RHIContext* rhiContext = nullptr;;

    std::string executablePath = "";
    std::string executableDirectory = "";
    std::string resourceDirectory = "";

    Application* app = nullptr;
};

EngineContext* hs_engine_create_context(const char* name, ERHIPlatform rhiPlatform);
void           hs_engine_destroy_context();

EngineContext* hs_engine_get_context();
void           hs_engine_set_context(EngineContext* context);
RHIContext*    hs_engine_get_rhi_context();


HS_NS_END

#endif /* EngineContext_hpp */
