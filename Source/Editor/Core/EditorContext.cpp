//
//  EditorContext.cpp
//  Editor
//
//  Central context for editor state management

#include "Precompile.h"
#include "EditorContext.h"

HS_NS_EDITOR_BEGIN

EditorContext& EditorContext::Get()
{
    static EditorContext instance;
    return instance;
}

void EditorContext::Finalize()
{
    _activeScene = nullptr;
    _selectedEntity = Entity();
    std::string().swap(_selectedAssetPath);
    std::string().swap(_currentAssetFolderPath);
    std::string().swap(_currentScenePath);
    _gizmoOperation = GizmoOperation::Translate;
    _gizmoSpace = GizmoSpace::World;
    _useSnap = false;
    _snapValue = 1.0f;
    _gizmoActive = false;
    _panelVisibility = PanelVisibility();
    _debugDrawSettings = DebugDrawSettings();
    std::vector<SelectionCallback>().swap(_selectionListeners);
}

void EditorContext::SetActiveScene(Scene* scene)
{
    _activeScene = scene;
    ClearSelection();

    if (!scene)
    {
        _currentScenePath.clear();
    }
}

void EditorContext::SetSelectedEntity(Entity entity)
{
    if (_selectedEntity != entity)
    {
        _selectedAssetPath.clear();
        _selectedEntity = entity;
        notifySelectionChanged();
    }
}

void EditorContext::SetSelectedAssetPath(const std::string& assetPath)
{
    if (_selectedAssetPath != assetPath || _selectedEntity.IsValid())
    {
        _selectedEntity = Entity();
        _selectedAssetPath = assetPath;
        notifySelectionChanged();
    }
}

void EditorContext::ClearSelectedAssetPath()
{
    if (!_selectedAssetPath.empty())
    {
        _selectedAssetPath.clear();
        notifySelectionChanged();
    }
}

void EditorContext::ClearSelection()
{
    if (_selectedEntity.IsValid() || !_selectedAssetPath.empty())
    {
        _selectedEntity = Entity();
        _selectedAssetPath.clear();
        notifySelectionChanged();
    }
}

void EditorContext::AddSelectionListener(SelectionCallback callback)
{
    _selectionListeners.push_back(std::move(callback));
}

void EditorContext::RemoveAllSelectionListeners()
{
    _selectionListeners.clear();
}

void EditorContext::notifySelectionChanged()
{
    for (const auto& callback : _selectionListeners)
    {
        callback(_selectedEntity);
    }
}

HS_NS_EDITOR_END
