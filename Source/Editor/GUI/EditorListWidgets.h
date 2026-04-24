//
//  EditorListWidgets.h
//  Editor
//

#ifndef __HS_EDITOR_EDITOR_LIST_WIDGETS_H__
#define __HS_EDITOR_EDITOR_LIST_WIDGETS_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorListWidgets
{
public:
    static ImVec4 GetSurfaceColor()
    {
        return ImGui::GetStyleColorVec4(ImGuiCol_Button);
    }

    static ImVec4 GetSurfaceHoverColor()
    {
        return ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
    }

    static ImVec4 GetSurfaceActiveColor()
    {
        return ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
    }

    static ImVec4 GetPrimaryTextColor()
    {
        return ImGui::GetStyleColorVec4(ImGuiCol_Text);
    }

    static ImVec4 GetSecondaryTextColor()
    {
        return ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
    }

    static ImVec4 GetTintedSurfaceColor(const ImVec4& tint, bool hovered)
    {
        const ImVec4 base = hovered ? GetSurfaceHoverColor() : GetSurfaceColor();
        const float blend = hovered ? 0.40f : 0.32f;
        return ImVec4(
            base.x + (tint.x - base.x) * blend,
            base.y + (tint.y - base.y) * blend,
            base.z + (tint.z - base.z) * blend,
            1.0f);
    }

    static bool SelectableRow(const char* id, const char* label, bool selected, ImGuiSelectableFlags flags = 0)
    {
        const float rowHeight = ImGui::GetTextLineHeightWithSpacing();
        const ImVec2 size(ImGui::GetContentRegionAvail().x, rowHeight);

        if (selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, GetSurfaceActiveColor());
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, GetSurfaceHoverColor());
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, GetSurfaceActiveColor());
        }

        const bool clicked = ImGui::Selectable(id, selected, flags, size);
        const ImVec2 rectMin = ImGui::GetItemRectMin();
        const ImVec2 rectMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(rectMin.x + ImGui::GetStyle().FramePadding.x, rectMin.y + ImGui::GetStyle().FramePadding.y),
            ImGui::ColorConvertFloat4ToU32(GetPrimaryTextColor()),
            label);

        if (selected)
        {
            ImGui::PopStyleColor(3);
        }

        return clicked;
    }
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_LIST_WIDGETS_H__ */
