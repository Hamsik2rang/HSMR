//
//  ResourcePanel.cpp
//  Editor
//
//  Asset browser panel implementation
//

#include "Editor/Panel/ResourcePanel.h"
#include "Editor/Asset/AssetDatabase.h"
#include "Editor/Core/EditorContext.h"
#include "Core/SystemContext.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "Resource/Model.h"
#include "Resource/Mesh.h"

#include "ImGui/imgui.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

HS_NS_EDITOR_BEGIN

// FontAwesome icons (fallback if not available)
#ifndef ICON_FA_FOLDER
#define ICON_FA_FOLDER "[Folder]"
#endif
#ifndef ICON_FA_FOLDER_OPEN
#define ICON_FA_FOLDER_OPEN "[Open]"
#endif
#ifndef ICON_FA_ARROW_LEFT
#define ICON_FA_ARROW_LEFT "<"
#endif
#ifndef ICON_FA_ARROW_RIGHT
#define ICON_FA_ARROW_RIGHT ">"
#endif
#ifndef ICON_FA_REDO
#define ICON_FA_REDO "R"
#endif
#ifndef ICON_FA_HOME
#define ICON_FA_HOME "H"
#endif

ResourcePanel::ResourcePanel(Window* window)
    : Panel(window)
{
}

ResourcePanel::~ResourcePanel()
{
}

bool ResourcePanel::Setup()
{
    // Initialize AssetDatabase with project's Assets folder
    hs::SystemContext* sysContext = hs::SystemContext::Get();
    if (sysContext && !sysContext->assetDirectory.empty())
    {
        AssetDatabase::Get().Initialize(sysContext->assetDirectory);
    }

    // Start at root
    navigateToFolder("");

    return true;
}

void ResourcePanel::Cleanup()
{
    AssetDatabase::Get().Shutdown();
}

void ResourcePanel::Draw()
{
    auto& vis = EditorContext::Get().GetPanelVisibility();
    if (!vis.resources)
    {
        return;
    }

    ImGui::Begin("Assets", &vis.resources);

    // Path bar at top
    drawPathBar();

    ImGui::Separator();

    // Main content area
    float contentWidth = ImGui::GetContentRegionAvail().x;

    if (_showFolderTree)
    {
        // Left: Folder tree
        ImGui::BeginChild("FolderTree", ImVec2(_folderTreeWidth, 0), true);
        drawFolderTree();
        ImGui::EndChild();

        // Splitter
        ImGui::SameLine();
        ImGui::Button("##Splitter", ImVec2(4.0f, -1));
        if (ImGui::IsItemActive())
        {
            _folderTreeWidth += ImGui::GetIO().MouseDelta.x;
            _folderTreeWidth = glm::clamp(_folderTreeWidth, 100.0f, contentWidth - 200.0f);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        ImGui::SameLine();
    }

    // Right: Asset list
    ImGui::BeginChild("AssetList", ImVec2(0, 0), true);
    drawAssetList();
    ImGui::EndChild();

    // Context menu
    drawContextMenu();

    ImGui::End();
}

void ResourcePanel::drawPathBar()
{
    // Navigation buttons
    bool canGoBack = _historyIndex > 0;
    bool canGoForward = _historyIndex < static_cast<int>(_pathHistory.size()) - 1;

    ImGui::BeginDisabled(!canGoBack);
    if (ImGui::Button(ICON_FA_ARROW_LEFT "##Back"))
    {
        navigateBack();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!canGoForward);
    if (ImGui::Button(ICON_FA_ARROW_RIGHT "##Forward"))
    {
        navigateForward();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_HOME "##Home"))
    {
        navigateToFolder("");
    }

    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_REDO "##Refresh"))
    {
        AssetDatabase::Get().Refresh();
    }

    ImGui::SameLine();

    // Breadcrumb path
    ImGui::Text("Assets");

    if (!_currentPath.empty())
    {
        std::string path = _currentPath;
        size_t start = 0;
        size_t end = 0;

        while ((end = path.find('/', start)) != std::string::npos || start < path.size())
        {
            if (end == std::string::npos)
                end = path.size();

            std::string segment = path.substr(start, end - start);
            std::string fullPath = path.substr(0, end);

            if (!segment.empty())
            {
                ImGui::SameLine();
                ImGui::Text("/");
                ImGui::SameLine();

                if (ImGui::SmallButton(segment.c_str()))
                {
                    navigateToFolder(fullPath);
                }
            }

            start = end + 1;
            if (start >= path.size())
                break;
        }
    }

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200);

    // Search box
    ImGui::SetNextItemWidth(200);
    ImGui::InputTextWithHint("##Search", "Search...", _searchBuffer, sizeof(_searchBuffer));
}

void ResourcePanel::drawFolderTree()
{
    const FolderEntry& root = AssetDatabase::Get().GetFolderTree();
    drawFolderTreeNode(root);
}

void ResourcePanel::drawFolderTreeNode(const FolderEntry& folder)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (folder.subFolders.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (folder.relativePath == _currentPath)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Root folder is always open
    if (folder.relativePath.empty())
    {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    const char* icon = (folder.relativePath == _currentPath) ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER;
    std::string displayName = folder.name.empty() ? "Assets" : folder.name;
    std::string label = std::string(icon) + " " + displayName;

    bool isOpen = ImGui::TreeNodeEx(label.c_str(), flags);

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        navigateToFolder(folder.relativePath);
    }

    if (isOpen)
    {
        for (const auto& subFolder : folder.subFolders)
        {
            drawFolderTreeNode(subFolder);
        }
        ImGui::TreePop();
    }
}

void ResourcePanel::drawAssetList()
{
    auto assets = AssetDatabase::Get().GetAssetsInFolder(_currentPath);
    auto subFolders = AssetDatabase::Get().GetSubFolders(_currentPath);

    // Filter by search
    std::string searchStr(_searchBuffer);
    for (auto& c : searchStr) c = static_cast<char>(std::tolower(c));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f));

    for (const auto& folderPath : subFolders)
    {
        size_t lastSlash = folderPath.rfind('/');
        std::string folderName = (lastSlash != std::string::npos)
            ? folderPath.substr(lastSlash + 1)
            : folderPath;
        if (folderName.empty())
        {
            folderName = folderPath;
        }

        // Apply search filter
        if (!searchStr.empty())
        {
            std::string lowerName = folderName;
            for (auto& c : lowerName) c = static_cast<char>(std::tolower(c));
            if (lowerName.find(searchStr) == std::string::npos)
                continue;
        }

        ImGui::PushID(folderPath.c_str());
        std::string label = std::string(ICON_FA_FOLDER) + " " + folderName;
        bool isSelected = (folderPath == _currentPath);
        if (ImGui::Selectable(label.c_str(), isSelected, 0, ImVec2(-1.0f, 0.0f)))
        {
            navigateToFolder(folderPath);
        }
        ImGui::PopID();
    }

    for (const auto* asset : assets)
    {
        // Apply search filter
        if (!searchStr.empty())
        {
            std::string lowerName = asset->name;
            for (auto& c : lowerName) c = static_cast<char>(std::tolower(c));
            if (lowerName.find(searchStr) == std::string::npos)
                continue;
        }

        drawAssetItem(asset);
    }

    ImGui::PopStyleVar();
}

void ResourcePanel::drawAssetItem(const AssetEntry* asset)
{
    ImGui::PushID(asset->relativePath.c_str());

    bool isSelected = (_selectedAssetPath == asset->relativePath);

    const char* icon = GetAssetTypeIcon(asset->type);
    const std::string& displayName = asset->name.empty() ? asset->relativePath : asset->name;
    std::string label = std::string(icon) + " " + displayName;
    if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(-1.0f, 0.0f)))
    {
        _selectedAssetPath = asset->relativePath;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        if (asset->type == EAssetType::Model)
        {
            Scene* scene = EditorContext::Get().GetActiveScene();
            if (scene)
            {
                hs::Model* model = AssetDatabase::Get().LoadModel(asset->relativePath);
                if (model)
                {
                    // Extract display name from asset name
                    std::string entityName = asset->name;
                    size_t dot = entityName.rfind('.');
                    if (dot != std::string::npos)
                        entityName = entityName.substr(0, dot);

                    Entity entity = scene->CreateEntity(entityName);
                    auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
                    meshRenderer.mesh = model->GetMesh();
                    if (model->GetMaterial())
                    {
                        meshRenderer.materials.push_back(model->GetMaterial());
                    }
                    if (meshRenderer.mesh)
                    {
                        const auto& bound = meshRenderer.mesh->GetBound();
                        meshRenderer.localBounds = AABB(glm::vec3(bound.min), glm::vec3(bound.max));
                        meshRenderer.boundsDirty = true;
                    }

                    EditorContext::Get().SetSelectedEntity(entity);
                }
            }
        }
    }

    // Drag source for drag & drop
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        const char* payloadType = nullptr;
        switch (asset->type)
        {
            case EAssetType::Model:   payloadType = "ASSET_MODEL"; break;
            case EAssetType::Texture: payloadType = "ASSET_TEXTURE"; break;
            default:                  payloadType = "ASSET_PATH"; break;
        }

        ImGui::SetDragDropPayload(payloadType, asset->relativePath.c_str(),
                                   asset->relativePath.size() + 1);

        // Drag preview
        ImGui::Text("%s %s", icon, displayName.c_str());

        ImGui::EndDragDropSource();
    }

    // Tooltip
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", displayName.c_str());
        ImGui::TextDisabled("Type: %s", GetAssetTypeName(asset->type));
        ImGui::TextDisabled("Size: %llu bytes", asset->fileSize);
        ImGui::EndTooltip();
    }

    ImGui::PopID();
}

void ResourcePanel::drawContextMenu()
{
    if (ImGui::BeginPopupContextWindow("AssetContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("Refresh"))
        {
            AssetDatabase::Get().Refresh();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Show in Explorer"))
        {
            std::string folderPath = AssetDatabase::Get().GetRootPath();
            if (!_currentPath.empty())
            {
                folderPath += "/" + _currentPath;
            }
#if defined(__APPLE__)
            std::string cmd = "open \"" + folderPath + "\"";
            system(cmd.c_str());
#elif defined(_WIN32)
            std::string cmd = "explorer \"" + folderPath + "\"";
            system(cmd.c_str());
#endif
        }

        ImGui::EndPopup();
    }
}

void ResourcePanel::navigateToFolder(const std::string& path)
{
    if (_currentPath == path)
        return;

    // Add to history
    if (_historyIndex < static_cast<int>(_pathHistory.size()) - 1)
    {
        // Remove forward history
        _pathHistory.resize(_historyIndex + 1);
    }

    _pathHistory.push_back(path);
    _historyIndex = static_cast<int>(_pathHistory.size()) - 1;

    _currentPath = path;
    _selectedAssetPath.clear();
}

void ResourcePanel::navigateBack()
{
    if (_historyIndex > 0)
    {
        _historyIndex--;
        _currentPath = _pathHistory[_historyIndex];
        _selectedAssetPath.clear();
    }
}

void ResourcePanel::navigateForward()
{
    if (_historyIndex < static_cast<int>(_pathHistory.size()) - 1)
    {
        _historyIndex++;
        _currentPath = _pathHistory[_historyIndex];
        _selectedAssetPath.clear();
    }
}

HS_NS_EDITOR_END
