#include "Editor/Core/EditorWindow.h"

#include "Engine/RendererPass/ForwardPass/ForwardOpaquePass.h"
#include "Engine/Renderer/Swapchain.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/GUI/GUIRenderer.h"

#include "Editor/Panel/Panel.h"
#include "Editor/Panel/DockspacePanel.h"

HS_NS_EDITOR_BEGIN

EditorWindow::EditorWindow(const char* name, uint32 width, uint32 height, uint64 flags)
    : Window(name, width, height, flags)
{
}

EditorWindow::~EditorWindow()
{
}

void EditorWindow::Render()
{
//    onRender();
    //    _renderer->Render(...);

    onRenderGUI();
}

bool EditorWindow::onInitialize()
{
//    _renderer = new Renderer();
//    _renderer->Initialize(&_nativeHandle);

    _guiRenderer = new GUIRenderer();
    _guiRenderer->Initialize(&_nativeHandle);

    _basePanel = new DockspacePanel();
    _basePanel->Setup();

    //    _renderer->AddPass(new ForwardOpaquePass("Opaque Pass", _renderer, 0));
}

void EditorWindow::onNextFrame()
{
//    _renderer->NextFrame();
    _guiRenderer->NextFrame(_swapchain);
}

void EditorWindow::onUpdate()
{
    //....
}

void EditorWindow::onRender()
{
    // 1. Render Scene
    //_renderer->Render({}, nullptr);
}

void EditorWindow::onPresent()
{
    _guiRenderer->Present(_swapchain);
}
void EditorWindow::onShutdown()
{
    if (nullptr != _guiRenderer)
    {
        _guiRenderer->Shutdown();
        delete _guiRenderer;
        _guiRenderer = nullptr;
    }
}

void EditorWindow::onRenderGUI()
{
    _basePanel->Draw(); // Draw panel tree.
    
//    _guiRenderer->Render({}, _swapchain->);
}

HS_NS_EDITOR_END
