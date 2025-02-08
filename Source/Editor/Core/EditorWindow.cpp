#include "Editor/Core/EditorWindow.h"

#include "Editor/GUI/GUIRenderer.h"

HS_NS_EDITOR_BEGIN

EditorWindow::EditorWindow(const char* name, uint32 width, uint32 height, uint64 flags)
    : Window(name, width, height, flags)
{}
EditorWindow::~EditorWindow()
{
}
void EditorWindow::NextFrame()
{
}
void EditorWindow::Update()
{
}
void EditorWindow::Render()
{
}
void EditorWindow::Present()
{
}
bool EditorWindow::onInitialize()
{
}
void EditorWindow::onNextFrame()
{
}
void EditorWindow::onUpdate()
{
}
void EditorWindow::onRender()
{
}
void EditorWindow::onPresent()
{
}
void EditorWindow::onShutdown()
{
}

HS_NS_EDITOR_END
