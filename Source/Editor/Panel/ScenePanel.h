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

#include "Engine/Renderer/RenderTarget.h"
#include "Engine/Geometry/GeometryDefinition.h"

#include "Engine/Camera.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ScenePanel : public Panel
{
public:
    ScenePanel(Window* window)
        : Panel(window)
        , _resolution(800, 600)
    {}
	~ScenePanel() override = default;

    bool Setup() override;
    void Cleanup() override;
    void Update(float deltaTime) override;
    
    void Draw() override;

    HS_FORCEINLINE void SetSceneRenderTarget(RenderTarget* renderTarget) { _currentRenderTarget = renderTarget; }

    HS_FORCEINLINE Resolution GetResolution() const { return _resolution; }

    HS_FORCEINLINE Camera* GetCamera() const { return _camera.get(); }

private:
    Resolution _resolution;
    RenderTarget* _currentRenderTarget;
    
    Scoped<Camera> _camera;

    uint16 _lastMouseX = 0;
    uint16 _lastMouseY = 0;
    bool _isMouseTracking = false;

    // View gizmo settings
    float _viewGizmoSize = 128.0f;
    float _viewGizmoMargin = 10.0f;

    void drawViewGizmo();
};

HS_NS_EDITOR_END

#endif /* ScenePanel_hpp */
