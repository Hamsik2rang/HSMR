//
//  ScenePanel.hpp
//  Editor
//
//  Created by Yongsik Im on 2/8/25.
//

#ifndef __HS_SCENE_PANEL_H__
#define __HS_SCENE_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"

#include "Renderer/RenderTarget.h"
#include "Resource/GeometryDefinition.h"

#include "Editor/Core/EditorCamera.h"
#include "Scene/Entity.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ScenePanel : public Panel
{
public:
    ScenePanel(Window* window)
        : Panel(window, "Scene")
        , _resolution(800, 600)
    {}
	~ScenePanel() override = default;

    bool Setup() override;
    void Cleanup() override;
    void Update(float deltaTime) override;

    void Draw() override;

    HS_FORCEINLINE void SetSceneRenderTarget(RenderTarget* renderTarget) { _currentRenderTarget = renderTarget; }

    HS_FORCEINLINE Resolution GetResolution() const { return _resolution; }

    HS_FORCEINLINE EditorCamera* GetEditorCamera() const { return _editorCamera.get(); }

    HS_FORCEINLINE ImVec2 GetViewportMin() const { return _viewportMin; }
    HS_FORCEINLINE ImVec2 GetViewportMax() const { return _viewportMax; }

private:
    Resolution _resolution;
    RenderTarget* _currentRenderTarget = nullptr;

    Scoped<EditorCamera> _editorCamera;

    uint16 _lastMouseX = 0;
    uint16 _lastMouseY = 0;
    bool _isMouseTracking = false;
    bool _rightClickStartedInViewport = false;  // Track if right-click started in this viewport

    // View gizmo settings
    float _viewGizmoSize = 76.8f;
    float _viewGizmoMargin = 10.0f;

    // Viewport bounds for mouse picking and gizmo
    ImVec2 _viewportMin;
    ImVec2 _viewportMax;
    bool _viewportHovered = false;
    bool _viewportFocused = false;
    float _deltaTime = 0.0f;

    void updateCameraControls(float deltaTime);
    void drawViewGizmo();
    void drawTransformGizmo();
    void handlePicking();

    // Ray generation for picking
    glm::vec3 screenToWorldRay(float mouseX, float mouseY);

    // Simple picking (requires mesh bounds)
    Entity pickEntity(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
};

HS_NS_EDITOR_END

#endif /* ScenePanel_hpp */
