#include "Engine/Application.h"

#include "Core/Log.h"
#include "Engine/Window.h"

HS_NS_BEGIN

Application::Application(const char* name, EngineContext* engineContext) noexcept
    : _name(name)
    , _engineContext(engineContext)
{

}

HS_NS_END
