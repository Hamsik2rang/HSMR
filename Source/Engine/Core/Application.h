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
#include "Engine/ThirdParty/ImGui/imgui.h"
#include "SDL3/SDL.h"

HS_NS_BEGIN

class Renderer;
class Scene;
class Window;

class Application
{
public:
    Application() {}
    virtual ~Application();
    
    virtual void Init(EngineContext* engineContext);
    virtual void Run();
    virtual void Shutdown();
    
protected:
    void update(float dt);
    void render();
    void renderGUI();
    
    Renderer* _renderer;
    Scene* _scene;
    Window* _window;
    
    bool _isInitialized = false;
};

HS_NS_END

#endif
