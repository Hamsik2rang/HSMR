#include "Editor/Core/EditorWindow.h"

#include "Engine/RendererPass/ForwardPass/ForwardOpaquePass.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/GUI/GUIRenderer.h"

HS_NS_EDITOR_BEGIN

EditorWindow::EditorWindow(const char* name, uint32 width, uint32 height, uint64 flags)
    : Window(name, width, height, flags)
{
}

EditorWindow::~EditorWindow()
{
}

bool EditorWindow::onInitialize()
{
    _renderer = new GUIRenderer();
    _renderer->Initialize(&_nativeHandle);

    _renderer->AddPass(new ForwardOpaquePass("Opaque Pass", _renderer, 0));
}
void EditorWindow::onNextFrame()
{
    _renderer->NextFrame();
}
void EditorWindow::onUpdate()
{
    //....
}
void EditorWindow::onRender()
{
    _renderer->Render({}, nullptr);
}

void EditorWindow::onPresent()
{
    _renderer->Present();
}
void EditorWindow::onShutdown()
{
    _renderer->Shutdown();
    delete _renderer;
}

HS_NS_EDITOR_END
