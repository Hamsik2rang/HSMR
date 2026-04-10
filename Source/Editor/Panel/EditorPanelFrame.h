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
    static bool BeginStandardPanel(const char* title, const EditorPanelWindowOptions& options = {})
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

    static void EndStandardPanel()
    {
        ImGui::End();
    }

    static bool BeginOverlayPanel(const char* title, ImGuiWindowFlags extraFlags = 0)
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

    static bool BeginPanelMenuBar()
    {
        return ImGui::BeginMenuBar();
    }

    static void EndPanelMenuBar()
    {
        ImGui::EndMenuBar();
    }

    static void BeginPanelContent(const EditorPanelContentOptions& options = {})
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

    static void EndPanelContent()
    {
        ImGui::EndChild();
        if (s_panelContentPaddingPushCount > 0)
        {
            ImGui::PopStyleVar();
            --s_panelContentPaddingPushCount;
        }
    }

private:
    static inline thread_local int s_panelContentPaddingPushCount = 0;
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_PANEL_FRAME_H__ */
