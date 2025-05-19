//
//  EditorApplication.h
//  HSMR
//
//  Created by Yongsik Im on 2/7/25.
//
#ifndef __HS_EDITOR_APPLICATION_H__
#define __HS_EDITOR_APPLICATION_H__

#include "Precompile.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/Window.h"

HS_NS_EDITOR_BEGIN

class GUIContext;

class HS_EDITOR_API EditorApplication : public Application
{
public:
    EditorApplication(const char* appName) noexcept;
    ~EditorApplication() override ;
    
    bool Initialize(EngineContext* engineContext) override;
    void Run() override;
    void Finalize() override;
    
private:
    GUIContext* _guiContext;
    
    bool _isInitialized = false;
    float _deltaTime = 0.0f;
};

HS_NS_EDITOR_END

#endif
