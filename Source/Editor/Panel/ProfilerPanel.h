//
//  ProfilerPanel.h
//  Editor
//
//  Tracy-integrated profiler panel
//

#pragma once

#include "Precompile.h"
#include "Editor/Panel/Panel.h"
#include "Engine/Camera.h"

#include <array>

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ProfilerPanel : public Panel
{
public:
    ProfilerPanel(Window* window);
    ~ProfilerPanel() override;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

    void SetSceneCamera(const Camera* camera) { _sceneCamera = camera; }

private:
    // UI sections
    void drawFrameTimeSection();
    void drawQuickStatsSection();
    void drawCameraSection();
    void drawTracySection();

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
    bool _showTracyHint = true;
    float _targetFPS = 60.0f;

    const Camera* _sceneCamera = nullptr;
};

HS_NS_EDITOR_END
