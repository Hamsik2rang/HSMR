#include "Engine/EngineContext.h"

#include "Core/Log.h"

HS_NS_BEGIN

EngineContext* g_engineContext;

EngineContext* GetEngineContext()
{
    if(nullptr == g_engineContext)
    {
        HS_LOG(crash, "EngineContext is not created!");
    }
    return g_engineContext;
}

EngineContext* CreateEngineContext(const char* name, ERHIPlatform rhiPlatform)
{
    g_engineContext = new EngineContext();
    g_engineContext->name = name;
    
    RHIContext* rhiContext = RHIContext::Create(rhiPlatform);
    g_engineContext->rhiContext = rhiContext;

    
}

HS_NS_END
