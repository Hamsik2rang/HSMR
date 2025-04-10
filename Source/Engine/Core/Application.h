//
//  Application.h
//  HSMR
//
//  Created by Yongsik Im on 2/2/25.
//
#ifndef __HS_APPLICATION_H__
#define __HS_APPLICATION_H__

#include "Precompile.h"

#include "Engine/Core/EngineContext.h"
#include "SDL3/SDL.h"

HS_NS_BEGIN

class Renderer;
class Scene;
class Window;

class Application
{
public:
    Application(const std::string& appName)
        : _name(appName)
    {}
    virtual ~Application() {}

    virtual bool Initialize(EngineContext* engineContext) = 0;
    virtual void Run()                                    = 0;
    virtual void Finalize()                               = 0;

protected:
    std::string _name;

    EngineContext* _engineContext;
    Window*        _window;
    bool           _isInitialized = false;
};

HS_NS_END

#endif
