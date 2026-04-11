//
//  EditorFormLayout.h
//  Editor
//

#ifndef __HS_EDITOR_EDITOR_FORM_LAYOUT_H__
#define __HS_EDITOR_EDITOR_FORM_LAYOUT_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorFormLayout
{
public:
    static bool Begin(const char* id, float labelWidth = 130.0f, ImGuiTableFlags extraFlags = 0)
    {
        ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings | extraFlags;
        if (!ImGui::BeginTable(id, 2, flags))
        {
            return false;
        }

        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        return true;
    }

    static void End()
    {
        ImGui::EndTable();
    }

    static void BeginRow(const char* label)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);
    }

    static bool CheckboxRow(const char* label, bool* value)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::Checkbox(id.c_str(), value);
    }

    static bool InputIntRow(const char* label, int* value, ImGuiInputTextFlags flags = 0)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::InputInt(id.c_str(), value, 1, 100, flags);
    }

    static bool DragFloatRow(
        const char* label,
        float* value,
        float speed = 1.0f,
        float min = 0.0f,
        float max = 0.0f,
        const char* format = "%.3f",
        ImGuiSliderFlags flags = 0)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::DragFloat(id.c_str(), value, speed, min, max, format, flags);
    }

    static bool DragFloat3Row(
        const char* label,
        float* value,
        float speed = 1.0f,
        float min = 0.0f,
        float max = 0.0f,
        const char* format = "%.3f",
        ImGuiSliderFlags flags = 0)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::DragFloat3(id.c_str(), value, speed, min, max, format, flags);
    }

    static bool DragFloat4Row(
        const char* label,
        float* value,
        float speed = 1.0f,
        float min = 0.0f,
        float max = 0.0f,
        const char* format = "%.3f",
        ImGuiSliderFlags flags = 0)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::DragFloat4(id.c_str(), value, speed, min, max, format, flags);
    }

    static bool DragIntRow(const char* label, int* value, float speed = 1.0f)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::DragInt(id.c_str(), value, speed);
    }

    static bool ComboRow(const char* label, int* currentItem, const char* const items[], int itemsCount)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::Combo(id.c_str(), currentItem, items, itemsCount);
    }

    static bool ColorEdit3Row(const char* label, float* color, ImGuiColorEditFlags flags = ImGuiColorEditFlags_Float)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::ColorEdit3(id.c_str(), color, flags);
    }

    static bool InputTextRow(const char* label, char* buffer, size_t bufferSize, ImGuiInputTextFlags flags = 0)
    {
        BeginRow(label);
        std::string id = std::string("##") + label;
        return ImGui::InputText(id.c_str(), buffer, bufferSize, flags);
    }
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_FORM_LAYOUT_H__ */
