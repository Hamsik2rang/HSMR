//
//  SceneStatusPanel.cpp
//  HSMR
//
//  Scene status overlay — frame stats + camera info
//

#include "Editor/Panel/SceneStatusPanel.h"
#include "Editor/Core/EditorContext.h"

#include "Core/HAL/Timer.h"
#include "Core/Profiler/Profiler.h"
#include "Editor/GUI/ImGuiExtension.h"

HS_NS_EDITOR_BEGIN

SceneStatusPanel::SceneStatusPanel(Window* window)
    : Panel(window)
{
    _frameTimeHistory.fill(0.0f);
}

SceneStatusPanel::~SceneStatusPanel()
{
}

bool SceneStatusPanel::Setup()
{
    Timer::Start();
    _lastFrameTime = Timer::GetElapsedMilliseconds();
    return true;
}

void SceneStatusPanel::Cleanup()
{
    Timer::Stop();
}

void SceneStatusPanel::Draw()
{
    HS_PROFILE_ZONE_NC("SceneStatusPanel::Draw", HS::Profile::ColorUI);

    auto& vis = EditorContext::Get().GetPanelVisibility();
    if (!vis.sceneStatus)
    {
        return;
    }

    // Calculate frame time
    double currentTime = Timer::GetElapsedMilliseconds();
    float deltaMs = static_cast<float>(currentTime - _lastFrameTime);
    _lastFrameTime = currentTime;

    // Update history
    _frameTimeHistory[_historyIndex] = deltaMs;
    _historyIndex = (_historyIndex + 1) % HISTORY_SIZE;

    // Update statistics
    _frameTimeSum += deltaMs;
    _frameCount++;
    _minFrameTime = std::min(_minFrameTime, deltaMs);
    _maxFrameTime = std::max(_maxFrameTime, deltaMs);

    if (_frameCount >= 60)
    {
        _avgFrameTime = _frameTimeSum / static_cast<float>(_frameCount);
        _frameTimeSum = 0.0f;
        _frameCount = 0;
    }

    // Plot the frame time to Tracy
    HS_PROFILE_PLOT("Frame Time (ms)", deltaMs);

    // Draw overlay window
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoTitleBar;

    const float padding = 10.0f;

    // Clamp position to scene bounds BEFORE Begin() to avoid conflicting with drag input
    bool hasBounds = (_sceneBoundsMax.x > _sceneBoundsMin.x) &&
                     (_sceneBoundsMax.y > _sceneBoundsMin.y);

    if (hasBounds && _prevWindowSize.x > 0.0f)
    {
        float clampedX = std::max(_sceneBoundsMin.x,
                         std::min(_prevWindowPos.x, _sceneBoundsMax.x - _prevWindowSize.x));
        float clampedY = std::max(_sceneBoundsMin.y,
                         std::min(_prevWindowPos.y, _sceneBoundsMax.y - _prevWindowSize.y));

        ImGui::SetNextWindowPos(ImVec2(clampedX, clampedY));
    }
    else
    {
        ImGui::SetNextWindowPos(ImVec2(padding, padding), ImGuiCond_Once);
    }

    ImGui::SetNextWindowBgAlpha(0.20f);

    if (ImGui::Begin("Scene Status", nullptr, windowFlags))
    {
        _prevWindowPos = ImGui::GetWindowPos();
        _prevWindowSize = ImGui::GetWindowSize();

        // Quick Stats
        _drawQuickStatsSection();

        ImGui::Separator();

        // Frame Time Graph
        if (_showFrameGraph)
        {
            _drawFrameTimeSection();
            ImGui::Separator();
        }

        // Camera Info
        if (_showCamera && _sceneCamera)
        {
            _drawCameraSection();
            ImGui::Separator();
        }

        // Toggle options
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("SceneStatusOptions");
        }

        if (ImGui::BeginPopup("SceneStatusOptions"))
        {
            ImGui::Checkbox("Frame Graph", &_showFrameGraph);
            ImGui::Checkbox("Camera Info", &_showCamera);
            ImGui::Separator();
            ImGui::SliderFloat("Target FPS", &_targetFPS, 30.0f, 144.0f, "%.0f");
            if (ImGui::Button("Reset Stats"))
            {
                _minFrameTime = 1000.0f;
                _maxFrameTime = 0.0f;
                _avgFrameTime = 0.0f;
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void SceneStatusPanel::_drawQuickStatsSection()
{
    float currentFPS = _avgFrameTime > 0.0f ? 1000.0f / _avgFrameTime : 0.0f;
    float targetMs = 1000.0f / _targetFPS;

    // Color based on performance
    ImVec4 fpsColor;
    if (_avgFrameTime <= targetMs)
    {
        fpsColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Green - Good
    }
    else if (_avgFrameTime <= targetMs * 1.5f)
    {
        fpsColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow - Warning
    }
    else
    {
        fpsColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red - Bad
    }

    ImGui::TextColored(fpsColor, "%.1f FPS", currentFPS);
    ImGui::SameLine();
    ImGui::Text("(%.2f ms)", _avgFrameTime);

    //ImGui::Text("Min: %.2f ms | Max: %.2f ms", _minFrameTime, _maxFrameTime);
}

void SceneStatusPanel::_drawFrameTimeSection()
{
    // Reorder history for continuous display
    float orderedHistory[HISTORY_SIZE];
    for (int i = 0; i < HISTORY_SIZE; ++i)
    {
        int idx = (_historyIndex + i) % HISTORY_SIZE;
        orderedHistory[i] = _frameTimeHistory[idx];
    }

    float targetMs = 1000.0f / _targetFPS;

    // Draw frame time graph
    char overlay[32];
    snprintf(overlay, sizeof(overlay), "Target: %.1f ms", targetMs);

    ImGui::PlotLines(
        "##FrameTime",
        orderedHistory,
        HISTORY_SIZE,
        0,
        overlay,
        0.0f,
        targetMs * 2.0f,  // Scale to 2x target for visibility
        ImVec2(230, 50)
    );
}

void SceneStatusPanel::_drawCameraSection()
{
    if (!_sceneCamera) return;

    const auto& pos = _sceneCamera->GetPosition();
    const auto& rot = _sceneCamera->GetRotation();
    const auto& forward = _sceneCamera->GetForward();

    ImGui::Text("Camera");
    ImGui::Text("  Pos: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
    ImGui::Text("  Rot: %.1f, %.1f, %.1f", rot.x, rot.y, rot.z);
    ImGui::Text("  Fwd: %.2f, %.2f, %.2f", forward.x, forward.y, forward.z);
}

HS_NS_EDITOR_END
