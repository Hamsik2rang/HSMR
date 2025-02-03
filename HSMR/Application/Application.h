//
//  Application.h
//  MetalSamples
//
//  Created by Yongsik Im on 2/2/25.
//
#ifndef __HS_APPLICATION_H__
#define __HS_APPLICATION_H__

class HSRenderer;
class HSScene;

#include "ImGui/imgui.h"
#include "SDL3/SDL.h"

class Application
{
public:
    void Init();
    void Run();
    void Shutdown();
    
private:
    void update(float dt);
    void render();
    void renderGUI();
    
    HSRenderer* _renderer;
    HSScene* _scene;
};


#endif
