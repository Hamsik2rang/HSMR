//
//  EditorWindow.h
//  Editor
//
//  Created by Yongsik Im on 2/7/25.
//

#ifndef __HS_EDITOR_WINDOW_H__
#define __HS_EDITOR_WINDOW_H__

#include "Precompile.h"

#include "Engine/Core/Window.h"

HS_NS_EDITOR_BEGIN

class Panel;

class GUIRenderer;


class HS_EDITOR_API EditorWindow : public Window
{
public:
    EditorWindow(const char* name, uint32 width, uint32 height, uint64 flags);
    ~EditorWindow() override;
    
    void Render() override;
    
private:
    bool onInitialize() override;
    void onNextFrame() override;
    void onUpdate() override;
    void onRender() override;
    void onPresent() override;
    void onShutdown() override;
    void onRenderGUI();
    
    
    GUIRenderer* _guiRenderer;
    Panel* _basePanel;
};



HS_NS_EDITOR_END

#endif /* EditorWindow_h */
