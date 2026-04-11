//
//  EditorTreeWidgets.h
//  Editor
//

#ifndef __HS_EDITOR_EDITOR_TREE_WIDGETS_H__
#define __HS_EDITOR_EDITOR_TREE_WIDGETS_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

#include <string>

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorTreeWidgets
{
public:
    static std::string MakeIconLabel(const char* icon, const std::string& text)
    {
        if (!icon || icon[0] == '\0')
        {
            return text;
        }
        return std::string(icon) + " " + text;
    }

    static bool BeginNode(
        const char* label,
        bool selected = false,
        bool leaf = false,
        bool defaultOpen = false,
        ImGuiTreeNodeFlags extraFlags = 0)
    {
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            extraFlags;

        if (selected)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (leaf)
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (defaultOpen)
        {
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        }

        return ImGui::TreeNodeEx(label, flags);
    }

    static bool IsSelectionClick()
    {
        return ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();
    }

    static bool SearchBar(const char* id, const char* hint, char* buffer, size_t bufferSize)
    {
        ImGui::SetNextItemWidth(-1);
        return ImGui::InputTextWithHint(id, hint, buffer, bufferSize);
    }

    static void EmptyState(const char* text)
    {
        ImGui::TextDisabled("%s", text);
    }
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_TREE_WIDGETS_H__ */
