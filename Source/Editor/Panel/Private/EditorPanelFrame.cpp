#include "Editor/Panel/EditorPanelFrame.h"

HS_NS_EDITOR_BEGIN

namespace
{
thread_local int s_panelContentPaddingPushCount = 0;
}

bool EditorPanelFrame::BeginStandardPanel(const char* title, const EditorPanelWindowOptions& options)
{
    ImGuiWindowFlags flags = options.extraFlags;
    if (options.useMenuBar)
    {
        flags |= ImGuiWindowFlags_MenuBar;
    }
    if (options.noTitleBar)
    {
        flags |= ImGuiWindowFlags_NoTitleBar;
    }
    if (options.noScrollbar)
    {
        flags |= ImGuiWindowFlags_NoScrollbar;
    }
    if (options.noScrollWithMouse)
    {
        flags |= ImGuiWindowFlags_NoScrollWithMouse;
    }

    return ImGui::Begin(title, options.pOpen, flags);
}

void EditorPanelFrame::EndStandardPanel()
{
    ImGui::End();
}

bool EditorPanelFrame::BeginOverlayPanel(const char* title, ImGuiWindowFlags extraFlags)
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoTitleBar |
        extraFlags;

    return ImGui::Begin(title, nullptr, flags);
}

bool EditorPanelFrame::BeginPanelMenuBar()
{
    return ImGui::BeginMenuBar();
}

void EditorPanelFrame::EndPanelMenuBar()
{
    ImGui::EndMenuBar();
}

void EditorPanelFrame::BeginPanelContent(const EditorPanelContentOptions& options)
{
    bool pushedPadding = false;
    if (options.padding.x >= 0.0f || options.padding.y >= 0.0f)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, options.padding);
        pushedPadding = true;
    }
    s_panelContentPaddingPushCount += pushedPadding ? 1 : 0;

    ImGui::BeginChild(options.id, ImVec2(0, 0), options.border, options.extraFlags);
}

void EditorPanelFrame::EndPanelContent()
{
    ImGui::EndChild();
    if (s_panelContentPaddingPushCount > 0)
    {
        ImGui::PopStyleVar();
        --s_panelContentPaddingPushCount;
    }
}

HS_NS_EDITOR_END
