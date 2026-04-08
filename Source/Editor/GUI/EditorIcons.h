//
//  EditorIcons.h
//  Editor
//
//  Material Symbols based icon helpers for editor ImGui panels.
//

#ifndef __HS_EDITOR_EDITOR_ICONS_H__
#define __HS_EDITOR_EDITOR_ICONS_H__

#include "Precompile.h"

#include "ImGui/imgui.h"

#include <string>

HS_NS_EDITOR_BEGIN

namespace EditorIcons
{
static constexpr const char* Close           = "\xEE\x97\x8D"; // close, U+E5CD
static constexpr const char* Add             = "\xEE\x85\x85"; // add, U+E145
static constexpr const char* Back            = "\xEE\x97\x84"; // arrow_back, U+E5C4
static constexpr const char* Forward         = "\xEE\x97\x88"; // arrow_forward, U+E5C8
static constexpr const char* Home            = "\xEE\xA6\xB2"; // home, U+E9B2
static constexpr const char* Refresh         = "\xEE\x97\x95"; // refresh, U+E5D5
static constexpr const char* Folder          = "\xEE\x8B\x87"; // folder, U+E2C7
static constexpr const char* FolderOpen      = "\xEE\x8B\x88"; // folder_open, U+E2C8
static constexpr const char* Description     = "\xEE\xA1\xB3"; // description, U+E873
static constexpr const char* Image           = "\xEE\x8F\xB4"; // image, U+E3F4
static constexpr const char* ViewInAr        = "\xEE\xBF\x89"; // view_in_ar, U+EFC9
static constexpr const char* Camera          = "\xEE\x8E\xAF"; // camera, U+E3AF
static constexpr const char* PhotoCamera     = "\xEE\x90\x92"; // photo_camera, U+E412
static constexpr const char* LightMode       = "\xEE\x94\x98"; // light_mode, U+E518
static constexpr const char* Settings        = "\xEE\xA2\xB8"; // settings, U+E8B8
static constexpr const char* Check           = "\xEE\x97\x8A"; // check, U+E5CA
static constexpr const char* Visibility      = "\xEE\xA3\xB4"; // visibility, U+E8F4
static constexpr const char* Draft           = "\xEE\x99\xAD"; // draft, U+E66D
static constexpr const char* Article         = "\xEE\xBD\x82"; // article, U+EF42
static constexpr const char* Palette         = "\xEE\x90\x8A"; // palette, U+E40A
static constexpr const char* Texture         = "\xEE\x90\xA1"; // texture, U+E421
static constexpr const char* DataObject      = "\xEE\xAB\x93"; // data_object, U+EAD3
static constexpr const char* Code            = "\xEE\xA1\xAF"; // code, U+E86F
static constexpr const char* DeployedCode    = "\xEF\x9C\xA0"; // deployed_code, U+F720
static constexpr const char* Category        = "\xEE\x95\xB4"; // category, U+E574
} // namespace EditorIcons

namespace EditorWidgets
{
HS_FORCEINLINE ImVec2 MeasureIconButton()
{
    const float buttonSize = ImGui::GetFrameHeight();
    return ImVec2(buttonSize, buttonSize);
}

HS_FORCEINLINE bool IconButton(const char* icon)
{
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 32));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 64));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    const bool pressed = ImGui::Button(icon, MeasureIconButton());

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    return pressed;
}

HS_FORCEINLINE bool IconButton(const char* icon, const char* id)
{
    std::string label = std::string(icon) + "##" + id;
    return IconButton(label.c_str());
}

HS_FORCEINLINE bool IconButtonColored(const char* icon, ImU32 color)
{
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    const bool pressed = IconButton(icon);
    ImGui::PopStyleColor();
    return pressed;
}
} // namespace EditorWidgets

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_ICONS_H__ */
