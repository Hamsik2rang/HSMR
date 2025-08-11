#include "Core/Application.h"

#include "Core/Log.h"
#include "Core/Window.h"

HS_NS_BEGIN

Application* g_app;

Application* hs_get_application()
{
    return g_app;
}

Application::Application(const char* name) noexcept
    : _name(name)
{
    g_app = this;
}

HS_NS_END
