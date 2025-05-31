#include "Engine/Core/EngineContext.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/FileSystem.h"

#ifdef __APPLE__
    #include "Engine/RHI/Metal/RHIContextMetal.h"
#else
#include "Engine/RHI/Vulkan/RHIContextVulkan.h"
#endif
#include <string>

HS_NS_BEGIN

static EngineContext* s_engineContext;

EngineContext* hs_engine_create_context(const char* name, ERHIPlatform rhiPlatform)
{
    if (nullptr != s_engineContext)
    {
        return s_engineContext;
    }

    s_engineContext                      = new EngineContext();
    s_engineContext->name                = name;
    s_engineContext->rhiPlatform         = rhiPlatform;
    s_engineContext->executablePath      = FileSystem::GetExecutablePath();
    s_engineContext->executableDirectory = FileSystem::GetDirectory(s_engineContext->executablePath);
    s_engineContext->resourceDirectory   = FileSystem::GetDefaultResourceDirectory();

    switch (rhiPlatform)
    {
#ifdef __APPLE__
        case ERHIPlatform::METAL:
        {
            s_engineContext->rhiContext = new RHIContextMetal();
        }
        break;
#else
        case ERHIPlatform::VULKAN:
        {
            s_engineContext->rhiContext = new RHIContextVulkan();
        }
        break;
#endif
        default:
			HS_LOG(crash, "Unsupported RHI Platform!");
            break;
    }

    s_engineContext->rhiContext->Initialize();

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

void hs_engine_destroy_context()
{
    s_engineContext->rhiContext->Finalize();
    delete s_engineContext;
}

HS_NS_END
