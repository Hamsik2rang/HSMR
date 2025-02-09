#include "Editor/Core/EditorWindow.h"

#include "Engine/RendererPass/ForwardPass/ForwardOpaquePass.h"
#include "Engine/Renderer/Swapchain.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/GUI/GUIRenderer.h"

#include "Editor/Panel/Panel.h"
#include "Editor/Panel/DockspacePanel.h"
#include "Editor/Panel/MenuPanel.h"

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
    onRender();
}

bool EditorWindow::onInitialize()
{
    _guiRenderer = new GUIRenderer();
    _guiRenderer->Initialize(&_nativeHandle);

    _guiRenderer->AddPass(new ForwardOpaquePass("Opaque Pass", _guiRenderer, ERenderingOrder::OPAQUE));

    for (int i = 0; i < sizeof(_renderTextures) / sizeof(RenderTexture); i++)
    {
        _renderTextures[i].width = _width;
        _renderTextures[i].height = _height;
        _renderTextures[i].colorBuffers.resize(1);
    }

    setupPanels();

    //    _renderer->AddPass(new ForwardOpaquePass("Opaque Pass", _renderer, 0));
}

void EditorWindow::onNextFrame()
{
    _guiRenderer->NextFrame(_swapchain);
}

void EditorWindow::onUpdate()
{
    //....
}

void EditorWindow::onRender()
{

    RenderTexture rt;
    rt.width  = _swapchain->GetWidth();
    rt.height = _swapchain->GetHeight();
    // 1. Render Scene
    _guiRenderer->Render({}, &rt);

    onRenderGUI();
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
    //    _guiRenderer->Render({}, nullptr);

    _basePanel->Draw(); // Draw panel tree.

    _guiRenderer->RenderGUI();
}

void EditorWindow::setupPanels()
{
    _basePanel = new DockspacePanel(this);
    _basePanel->Setup();
    
    Panel* menuPanel = new MenuPanel(this);
    menuPanel->Setup();
    
    _basePanel->InsertPanel(menuPanel);

}

HS_NS_EDITOR_END
