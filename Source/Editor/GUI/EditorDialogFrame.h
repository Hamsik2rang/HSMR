//
//  EditorDialogFrame.h
//  Editor
//

#ifndef __HS_EDITOR_EDITOR_DIALOG_FRAME_H__
#define __HS_EDITOR_EDITOR_DIALOG_FRAME_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorDialogFrame
{
public:
    static bool BeginCenteredModal(
        const char* title,
        bool* pOpen,
        const ImVec2& initialSize = ImVec2(0.0f, 0.0f),
        ImGuiWindowFlags extraFlags = 0)
    {
        ImGui::OpenPopup(title);

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (initialSize.x > 0.0f || initialSize.y > 0.0f)
        {
            ImGui::SetNextWindowSize(initialSize, ImGuiCond_Appearing);
        }

        return ImGui::BeginPopupModal(title, pOpen, ImGuiWindowFlags_AlwaysAutoResize | extraFlags);
    }

    static void BeginFooterButtons(int buttonCount, float buttonWidth = 120.0f, float spacing = 10.0f)
    {
        const float totalWidth = buttonWidth * static_cast<float>(buttonCount) +
                                 spacing * static_cast<float>(buttonCount > 0 ? buttonCount - 1 : 0);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - totalWidth) * 0.5f);
    }

    static void EndModal()
    {
        ImGui::EndPopup();
    }
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_DIALOG_FRAME_H__ */
