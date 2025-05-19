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
#include "Engine/Renderer/RenderDefinition.h"

namespace HS
{
class RenderTarget;
class Renderer;
} // namespace HS

HS_NS_EDITOR_BEGIN

class Panel;

class HS_EDITOR_API EditorWindow : public Window
{
public:
    EditorWindow(const char* name, uint32 width, uint32 height, EWindowFlags flags);
    ~EditorWindow() override;

    void Render() override;

private:
    bool dispatchEvent(EWindowEvent event, uint32 windowID);
    void setupPanels();

    bool onInitialize() override;
    void onNextFrame() override;
    void onUpdate() override;
    void onRender() override;
    void onPresent() override;
    void onShutdown() override;
    void onRenderGUI();

    std::vector<RenderTarget> _renderTargets;

    Panel*    _basePanel;
    Panel*    _menuPanel;
    Panel*    _scenePanel;
};

HS_NS_EDITOR_END

#endif /* EditorWindow_h */
