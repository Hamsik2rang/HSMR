#include "Application.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Scene.h"
#include "Engine/Renderer/Renderer.h"

HS_NS_BEGIN

Application::~Application()
{
    if (_scene)
    {
        delete _scene;
    }
    if (_renderer)
    {
        delete _renderer;
    }
}

void Application::Init(EngineContext* engineContext)
{
    if (_isInitialized)
    {
        HS_LOG(error, "Application is already initialized");
        return;
    }

    //...
    
    _isInitialized = true;
}

HS_NS_END
