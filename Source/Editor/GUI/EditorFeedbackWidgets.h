//
//  EditorFeedbackWidgets.h
//  Editor
//

#ifndef __HS_EDITOR_EDITOR_FEEDBACK_WIDGETS_H__
#define __HS_EDITOR_EDITOR_FEEDBACK_WIDGETS_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorFeedbackWidgets
{
public:
    static void SecondaryText(const char* text)
    {
        ImGui::TextDisabled("%s", text);
    }

    static void EmptyState(const char* primary, const char* secondary = nullptr)
    {
        ImGui::TextDisabled("%s", primary);
        if (secondary && secondary[0] != '\0')
        {
            ImGui::TextDisabled("%s", secondary);
        }
    }
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_FEEDBACK_WIDGETS_H__ */
