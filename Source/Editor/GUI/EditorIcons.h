//
//  EditorIcons.h
//  Editor
//
//  Material Symbols based icon helpers for editor ImGui panels.
//

#ifndef __HS_EDITOR_EDITOR_ICONS_H__
#define __HS_EDITOR_EDITOR_ICONS_H__

#include "Precompile.h"

#include "Editor/GUI/MaterialSymbolsIcons.h"

#include "ImGui/imgui.h"

#include <string>

HS_NS_EDITOR_BEGIN

namespace EditorIcons
{
// Keep semantic aliases here so editor code can stay readable even if we ever swap icon sets.
// For direct icon lookup, use MaterialSymbolsIcons::Icon* from MaterialSymbolsIcons.h.
static constexpr const char* Close           = MaterialSymbolsIcons::IconClose;
static constexpr const char* Add             = MaterialSymbolsIcons::IconAdd;
static constexpr const char* Back            = MaterialSymbolsIcons::IconArrowBack;
static constexpr const char* Forward         = MaterialSymbolsIcons::IconArrowForward;
static constexpr const char* Home            = MaterialSymbolsIcons::IconHome;
static constexpr const char* Refresh         = MaterialSymbolsIcons::IconRefresh;
static constexpr const char* Folder          = MaterialSymbolsIcons::IconFolder;
static constexpr const char* FolderOpen      = MaterialSymbolsIcons::IconFolderOpen;
static constexpr const char* Description     = MaterialSymbolsIcons::IconDescription;
static constexpr const char* Image           = MaterialSymbolsIcons::IconImage;
static constexpr const char* ViewInAr        = MaterialSymbolsIcons::IconViewInAr;
static constexpr const char* Camera          = MaterialSymbolsIcons::IconCamera;
static constexpr const char* PhotoCamera     = MaterialSymbolsIcons::IconPhotoCamera;
static constexpr const char* LightMode       = MaterialSymbolsIcons::IconLightMode;
static constexpr const char* Settings        = MaterialSymbolsIcons::IconSettings;
static constexpr const char* Check           = MaterialSymbolsIcons::IconCheck;
static constexpr const char* Visibility      = MaterialSymbolsIcons::IconVisibility;
static constexpr const char* VisibilityOff   = MaterialSymbolsIcons::IconVisibilityOff;
static constexpr const char* Draft           = MaterialSymbolsIcons::IconDraft;
static constexpr const char* Article         = MaterialSymbolsIcons::IconArticle;
static constexpr const char* Palette         = MaterialSymbolsIcons::IconPalette;
static constexpr const char* Texture         = MaterialSymbolsIcons::IconTexture;
static constexpr const char* DataObject      = MaterialSymbolsIcons::IconDataObject;
static constexpr const char* Code            = MaterialSymbolsIcons::IconCode;
static constexpr const char* DeployedCode    = MaterialSymbolsIcons::IconDeployedCode;
static constexpr const char* Category        = MaterialSymbolsIcons::IconCategory;
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
