//
//  Application.h
//  HSMR
//
//  Created by Yongsik Im on 2025.2.7
//
#ifndef __HS_APPLICATION_H__
#define __HS_APPLICATION_H__

#include "Precompile.h"

#include "Engine/Core/EngineContext.h"

#include "Engine/Platform/PlatformApplication.h"

namespace HS
{
class Renderer;
}

HS_NS_BEGIN

class Renderer;
class Scene;
class Window;

class Application
{
public:
    Application(const char* appName) noexcept;
    virtual ~Application() = default;

    virtual bool Initialize(EngineContext* engineContext) = 0;
    virtual void Run()                                    = 0;
    virtual void Finalize()                               = 0;

protected:
    const char* _name;

    EngineContext* _engineContext;
    Window*        _window;
};

Application* hs_get_application();

HS_NS_END

#endif
