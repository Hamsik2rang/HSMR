//
//  EditorContext.h
//  Editor
//
//  Central context for editor state management (scene, selection, etc.)
//

#pragma once

#include "Precompile.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"

#include <functional>
#include <vector>

HS_NS_EDITOR_BEGIN

struct PanelVisibility
{
    bool scene       = true;
    bool hierarchy   = true;
    bool inspector   = true;
    bool resources   = true;
    bool sceneStatus = true;
    bool profiler    = false;
};

struct DebugDrawSettings
{
    bool showDebugPass = true;
};

/**
 * @brief Editor context singleton
 *
 * Manages shared editor state:
 * - Active scene
 * - Selected entity
 * - Gizmo operation mode
 * - Panel visibility
 */
class HS_EDITOR_API EditorContext
{
public:
    // Gizmo operation modes
    enum class GizmoOperation
    {
        Translate,
        Rotate,
        Scale
    };

    // Gizmo coordinate space
    enum class GizmoSpace
    {
        Local,
        World
    };

    // Singleton access
    static EditorContext& Get();

    // ===== Scene Management =====

    void SetActiveScene(Scene* scene);
    Scene* GetActiveScene() const { return _activeScene; }

    // ===== Selection =====

    void SetSelectedEntity(Entity entity);
    Entity GetSelectedEntity() const { return _selectedEntity; }
    void ClearSelection();
    bool HasSelection() const { return _selectedEntity.IsValid(); }

    // Selection change callback
    using SelectionCallback = std::function<void(Entity)>;
    void AddSelectionListener(SelectionCallback callback);
    void RemoveAllSelectionListeners();

    // ===== Gizmo State =====

    void SetGizmoOperation(GizmoOperation op) { _gizmoOperation = op; }
    GizmoOperation GetGizmoOperation() const { return _gizmoOperation; }

    void SetGizmoSpace(GizmoSpace space) { _gizmoSpace = space; }
    GizmoSpace GetGizmoSpace() const { return _gizmoSpace; }

    void SetUseSnap(bool useSnap) { _useSnap = useSnap; }
    bool GetUseSnap() const { return _useSnap; }

    void SetSnapValue(float value) { _snapValue = value; }
    float GetSnapValue() const { return _snapValue; }

    // ===== State Flags =====

    void SetGizmoActive(bool active) { _gizmoActive = active; }
    bool IsGizmoActive() const { return _gizmoActive; }

    // ===== Panel Visibility =====

    PanelVisibility& GetPanelVisibility() { return _panelVisibility; }

    DebugDrawSettings& GetDebugDrawSettings() { return _debugDrawSettings; }

private:
    EditorContext() = default;
    ~EditorContext() = default;
    EditorContext(const EditorContext&) = delete;
    EditorContext& operator=(const EditorContext&) = delete;

    void notifySelectionChanged();

    // State
    Scene* _activeScene = nullptr;
    Entity _selectedEntity;

    // Gizmo
    GizmoOperation _gizmoOperation = GizmoOperation::Translate;
    GizmoSpace _gizmoSpace = GizmoSpace::World;
    bool _useSnap = false;
    float _snapValue = 1.0f;
    bool _gizmoActive = false;

    // Panel visibility
    PanelVisibility _panelVisibility;
    DebugDrawSettings _debugDrawSettings;

    // Listeners
    std::vector<SelectionCallback> _selectionListeners;
};

HS_NS_EDITOR_END
