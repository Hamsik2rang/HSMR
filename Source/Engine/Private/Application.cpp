#include "Engine/Application.h"

#include "Core/SystemContext.h"

#include "RHI/RHIContext.h"

HS_NS_BEGIN

Application* g_AppIntance = nullptr;

Application::Application(const char* appName) noexcept
    : _name(appName)
    , _window(nullptr)
    , _rhiContext(nullptr)
{
    init();
}

void Application::init()
{
    if (g_AppIntance != nullptr)
    {
        HS_LOG(crash, "Application instance already exists");
    }

    SystemContext::Init();

#if __WINDOWS__
    _rhiContext = RHIContext::Create(ERHIPlatform::VULKAN);
#else
    _rhiContext = RHIContext::Create(ERHIPlatform::METAL);
#endif
    g_AppIntance = this;
}

Application* Application::Get()
{
    if (g_AppIntance == nullptr)
    {
        HS_LOG(crash, "Application instance is null");
    }

    return g_AppIntance;
}


HS_NS_END