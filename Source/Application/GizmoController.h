//
//  GizmoController.h
//  HSMR
//
//  ImGuizmo wrapper for 3D object manipulation
//
#ifndef __HS_GIZMO_GIZMO_CONTROLLER_H__
#define __HS_GIZMO_GIZMO_CONTROLLER_H__

#include "Precompile.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

// Forward declarations
class Camera;
class SceneObject;

// Gizmo manipulation modes
class HS_APPLICATION_API GizmoController
{
public:
    enum class Mode
    {
        Translate,
        Rotate,
        Scale
    };

    enum class Space
    {
        Local,
        World
    };

    GizmoController() = default;
    ~GizmoController() = default;

    // Mode control
    void SetMode(Mode mode) { _mode = mode; }
    Mode GetMode() const { return _mode; }

    void SetSpace(Space space) { _space = space; }
    Space GetSpace() const { return _space; }

    // Snap settings
    void SetTranslateSnap(float snap) { _translateSnap = snap; }
    void SetRotateSnap(float snap) { _rotateSnap = snap; }
    void SetScaleSnap(float snap) { _scaleSnap = snap; }

    float GetTranslateSnap() const { return _translateSnap; }
    float GetRotateSnap() const { return _rotateSnap; }
    float GetScaleSnap() const { return _scaleSnap; }

    void EnableSnap(bool enable) { _snapEnabled = enable; }
    bool IsSnapEnabled() const { return _snapEnabled; }

    // Process keyboard input (W=Translate, E=Rotate, R=Scale)
    void ProcessInput();

    // Manipulate an object
    // Returns true if the transform was modified
    bool Manipulate(Camera* camera, SceneObject* object);

    // Check if gizmo is being used
    bool IsUsing() const { return _isUsing; }
    bool IsOver() const { return _isOver; }

    // Draw options
    void SetEnabled(bool enabled) { _enabled = enabled; }
    bool IsEnabled() const { return _enabled; }

    void SetDrawGrid(bool draw) { _drawGrid = draw; }
    bool GetDrawGrid() const { return _drawGrid; }

    void SetGridSize(float size) { _gridSize = size; }
    float GetGridSize() const { return _gridSize; }

    // Draw helpers
    void DrawGrid(Camera* camera, float gridSize = 10.0f);
    void DrawViewManipulator(Camera* camera, float size = 128.0f);

    // Get manipulation delta (for undo/redo)
    const glm::mat4& GetDeltaMatrix() const { return _deltaMatrix; }

private:
    Mode _mode = Mode::Translate;
    Space _space = Space::World;

    float _translateSnap = 0.5f;
    float _rotateSnap = 15.0f;  // degrees
    float _scaleSnap = 0.1f;
    bool _snapEnabled = false;

    bool _enabled = true;
    bool _isUsing = false;
    bool _isOver = false;

    bool _drawGrid = true;
    float _gridSize = 10.0f;

    glm::mat4 _deltaMatrix = glm::mat4(1.0f);
};

HS_NS_END

#endif // __HS_GIZMO_GIZMO_CONTROLLER_H__
