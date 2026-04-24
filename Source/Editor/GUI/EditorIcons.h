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
#include "ImGui/imgui_internal.h"

#include <algorithm>
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

HS_FORCEINLINE ImVec2 MeasureIconButtonSmall()
{
    const float buttonSize = std::max(13.0f, ImGui::GetFrameHeight() - 6.0f);
    return ImVec2(buttonSize, buttonSize);
}

HS_FORCEINLINE ImVec2 MeasureIconButtonHeader()
{
    const float buttonSize = std::max(14.0f, ImGui::GetFrameHeight() - 6.0f);
    return ImVec2(buttonSize, buttonSize);
}

HS_FORCEINLINE ImVec2 MeasureIconButtonMenuBar()
{
    const float buttonSize = std::max(13.0f, ImGui::GetFrameHeight() - 8.0f);
    return ImVec2(buttonSize, buttonSize);
}

HS_FORCEINLINE bool IconButtonEx(
    const char* label,
    const ImVec2& buttonSize,
    ImU32 buttonColor = IM_COL32(0, 0, 0, 0),
    ImU32 hoveredColor = IM_COL32(255, 255, 255, 32),
    ImU32 activeColor = IM_COL32(255, 255, 255, 64))
{
    ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    const bool pressed = ImGui::Button(label, buttonSize);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    return pressed;
}

HS_FORCEINLINE bool IconButton(const char* icon)
{
    return IconButtonEx(icon, MeasureIconButton());
}

HS_FORCEINLINE bool IconButton(const char* icon, const char* id)
{
    std::string label = std::string(icon) + "##" + id;
    return IconButton(label.c_str());
}

HS_FORCEINLINE bool IconButtonSmall(const char* icon, const char* id)
{
    std::string label = std::string(icon) + "##" + id;
    return IconButtonEx(label.c_str(), MeasureIconButtonSmall());
}

HS_FORCEINLINE bool IconButtonHeader(const char* icon, const char* id)
{
    std::string label = std::string(icon) + "##" + id;
    return IconButtonEx(label.c_str(), MeasureIconButtonHeader());
}

HS_FORCEINLINE bool IconButtonMenuBar(
    const char* icon,
    const char* id,
    ImU32 buttonColor = IM_COL32(0, 0, 0, 0),
    ImU32 hoveredColor = IM_COL32(255, 255, 255, 32),
    ImU32 activeColor = IM_COL32(255, 255, 255, 64))
{
    std::string label = std::string(icon) + "##" + id;
    return IconButtonEx(label.c_str(), MeasureIconButtonMenuBar(), buttonColor, hoveredColor, activeColor);
}

HS_FORCEINLINE bool IconButtonColored(const char* icon, ImU32 color)
{
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    const bool pressed = IconButton(icon);
    ImGui::PopStyleColor();
    return pressed;
}

HS_FORCEINLINE void RightAlignNextItem(float width, float rightPadding = 0.0f)
{
    float cursorX = ImGui::GetWindowContentRegionMax().x - width - rightPadding;
    if (cursorX > ImGui::GetCursorPosX())
    {
        ImGui::SetCursorPosX(cursorX);
    }
}

HS_FORCEINLINE bool SearchFieldRightAligned(
    const char* id,
    const char* hint,
    char* buffer,
    size_t bufferSize,
    float preferredWidth = 200.0f)
{
    float availWidth = ImGui::GetContentRegionAvail().x;
    if (availWidth > preferredWidth + ImGui::GetStyle().ItemSpacing.x)
    {
        ImGui::SameLine();
        RightAlignNextItem(preferredWidth);
        ImGui::SetNextItemWidth(preferredWidth);
    }
    else
    {
        ImGui::SetNextItemWidth(-1.0f);
    }

    return ImGui::InputTextWithHint(id, hint, buffer, bufferSize);
}

HS_FORCEINLINE bool BeginRemovableSectionHeader(const char* label, const char* removeId, bool& outRemove)
{
    const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
    const bool open = ImGui::CollapsingHeader(label, flags);

    const ImVec2 headerMin = ImGui::GetItemRectMin();
    const ImVec2 headerMax = ImGui::GetItemRectMax();
    const ImVec2 restoreCursor = ImGui::GetCursorScreenPos();

    const ImVec2 buttonSize = MeasureIconButtonHeader();
    const float buttonPaddingX = ImGui::GetStyle().FramePadding.x;
    const float buttonPosX = headerMax.x - buttonSize.x - buttonPaddingX;
    const float headerHeight = headerMax.y - headerMin.y;
    const float buttonPosY = headerMin.y + (headerHeight - buttonSize.y) * 0.5f;

    ImGui::SetCursorScreenPos(ImVec2(buttonPosX, buttonPosY));
    if (IconButtonHeader(EditorIcons::Close, removeId))
    {
        outRemove = true;
    }
    ImGui::SetCursorScreenPos(restoreCursor);

    return open;
}

HS_FORCEINLINE float DrawVerticalSplitter(
    const char* id,
    float currentLeadingWidth,
    float minLeadingWidth,
    float maxLeadingWidth,
    float splitterWidth = 4.0f)
{
    ImGui::Button(id, ImVec2(splitterWidth, -1.0f));
    if (ImGui::IsItemActive())
    {
        currentLeadingWidth += ImGui::GetIO().MouseDelta.x;
        currentLeadingWidth = std::clamp(currentLeadingWidth, minLeadingWidth, maxLeadingWidth);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }

    return currentLeadingWidth;
}
} // namespace EditorWidgets

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_EDITOR_ICONS_H__ */
