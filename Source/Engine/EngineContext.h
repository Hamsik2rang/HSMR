#pragma once
#ifndef __HS_ENGINE_CONTEXT_H__
#define __HS_ENGINE_CONTEXT_H__

#include "Precompile.h"

#include "Core/SystemContext.h"

#include "RHI/RHIDefinition.h"
#include "RHI/RHIContext.h"

HS_NS_BEGIN

struct HS_API EngineContext
{
    const char* name = "";
    
    ERHIPlatform rhiPlatform;
    RHIContext* rhiContext;

    std::string executablePath = "";
	std::string executableDirectory = "";
	std::string assetDirectory = "";
};

HS_API EngineContext* GetEngineContext();
HS_API EngineContext* CreateEngineContext(const char* name, ERHIPlatform rhiPlatform);

HS_NS_END


#endif
