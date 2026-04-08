//
//  ResourcePanel.h
//  Editor
//
//  Asset browser panel for project resources
//

#ifndef __HS_RESOURCE_PANEL_H__
#define __HS_RESOURCE_PANEL_H__

#include "Precompile.h"
#include "Editor/Panel/Panel.h"
#include "Editor/Asset/AssetTypes.h"

#include <string>
#include <vector>

HS_NS_EDITOR_BEGIN

struct AssetEntry;
struct FolderEntry;

class HS_EDITOR_API ResourcePanel : public Panel
{
public:
    ResourcePanel(Window* window);
    ~ResourcePanel() override;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

private:
    // UI Drawing
    void drawPathBar();
    void drawFolderTree();
    void drawFolderTreeNode(const FolderEntry& folder);
    void drawAssetList();
    void drawAssetItem(const AssetEntry* asset);
    void drawContextMenu();

    // Navigation
    void navigateToFolder(const std::string& path);
    void navigateBack();
    void navigateForward();

    // State
    std::string _currentPath;           // Current folder being viewed
    std::string _selectedAssetPath;     // Currently selected asset
    std::vector<std::string> _pathHistory;
    int _historyIndex = -1;

    // View settings
    float _thumbnailSize = 80.0f;
    bool _showFolderTree = true;
    float _folderTreeWidth = 200.0f;

    // Search
    char _searchBuffer[256] = {0};

    // Drag state
    bool _isDragging = false;
    std::string _draggedAssetPath;
};

HS_NS_EDITOR_END

#endif // __HS_RESOURCE_PANEL_H__
