#ifndef __HS_EDITOR_EDITOR_PANEL_FRAME_H__
#define __HS_EDITOR_EDITOR_PANEL_FRAME_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

struct HS_EDITOR_API EditorPanelWindowOptions
{
    bool* pOpen = nullptr;
    bool useMenuBar = false;
    bool noTitleBar = false;
    bool noScrollbar = false;
    bool noScrollWithMouse = false;
    ImGuiWindowFlags extraFlags = 0;
};

struct HS_EDITOR_API EditorPanelContentOptions
{
    const char* id = "PanelContent";
    bool border = false;
    ImVec2 padding = ImVec2(-1.0f, -1.0f);
    ImGuiWindowFlags extraFlags = 0;
};

class HS_EDITOR_API EditorPanelFrame
{
public:
    static bool BeginStandardPanel(const char* title, const EditorPanelWindowOptions& options = {});
    static void EndStandardPanel();

    static bool BeginOverlayPanel(const char* title, ImGuiWindowFlags extraFlags = 0);

    static bool BeginPanelMenuBar();
    static void EndPanelMenuBar();

    static void BeginPanelContent(const EditorPanelContentOptions& options = {});
    static void EndPanelContent();
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_PANEL_FRAME_H__ */
