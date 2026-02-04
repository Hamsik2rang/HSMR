#include "Engine/Application.h"

#include "Core/SystemContext.h"

#include "RHI/RHIContext.h"

HS_NS_BEGIN

Application::Application(const char* appName) noexcept
    : _name(appName)
    , _window(nullptr)
    , _rhiContext(nullptr)
{
    SystemContext::Init();

#if __WINDOWS__
    _rhiContext = RHIContext::Create(ERHIPlatform::VULKAN);
#else
    _rhiContext = RHIContext::Create(ERHIPlatform::METAL);
#endif
}


HS_NS_END