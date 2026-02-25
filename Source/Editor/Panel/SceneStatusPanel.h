//
//  SceneStatusPanel.h
//  Editor
//
//  Scene status overlay — frame stats + camera info
//

#ifndef __HS_EDITOR_SCENE_STATUS_PANEL_H__
#define __HS_EDITOR_SCENE_STATUS_PANEL_H__

#include "Precompile.h"
#include "Editor/Panel/Panel.h"
#include "Editor/Core/EditorCamera.h"

#include <array>

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API SceneStatusPanel : public Panel
{
public:
    SceneStatusPanel(Window* window);
    ~SceneStatusPanel() override;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

    void SetSceneCamera(const EditorCamera* camera) { _sceneCamera = camera; }
    void SetSceneBounds(ImVec2 boundsMin, ImVec2 boundsMax) { _sceneBoundsMin = boundsMin; _sceneBoundsMax = boundsMax; }

private:
    // UI sections
    void _drawFrameTimeSection();
    void _drawQuickStatsSection();
    void _drawCameraSection();

    // Frame time history
    static constexpr int HISTORY_SIZE = 128;
    std::array<float, HISTORY_SIZE> _frameTimeHistory{};
    int _historyIndex = 0;

    // Statistics
    double _lastFrameTime = 0.0;
    float _minFrameTime = 1000.0f;
    float _maxFrameTime = 0.0f;
    float _avgFrameTime = 0.0f;
    float _frameTimeSum = 0.0f;
    int _frameCount = 0;

    // Settings
    bool _showFrameGraph = true;
    bool _showCamera = true;
    float _targetFPS = 60.0f;

    const EditorCamera* _sceneCamera = nullptr;

    // Scene bounds for position clamping
    ImVec2 _sceneBoundsMin = ImVec2(0, 0);
    ImVec2 _sceneBoundsMax = ImVec2(0, 0);
    ImVec2 _prevWindowPos = ImVec2(0, 0);
    ImVec2 _prevWindowSize = ImVec2(0, 0);
};

HS_NS_EDITOR_END

#endif /* __HS_EDITOR_SCENE_STATUS_PANEL_H__ */
