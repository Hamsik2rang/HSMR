//
//  ProfilerPanel.cpp
//  HSMR
//
//  Created on 10/03/25.
//
#include "Editor/Panel/ProfilerPanel.h"

#include "Core/HAL/Timer.h"
#include "Editor/GUI/ImGuiExtension.h"

HS_NS_EDITOR_BEGIN

ProfilerPanel::ProfilerPanel(Window* window)
    : Panel(window)
{
}

ProfilerPanel::~ProfilerPanel()
{
}

bool ProfilerPanel::Setup()
{
    Timer::Start();

    return true;
}

void ProfilerPanel::Cleanup()
{
    Timer::Stop();
}

void ProfilerPanel::Draw()
{
    static bool open = true;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Profiler Overlay", &open, windowFlags))
    {
        double elapsedTime = Timer::GetElapsedMilliseconds();
        double delta       = elapsedTime - _lastFrameTime;
        ImGui::Text("Frame: %.1f FPS (%.3f ms/frame)", 1000.0f / delta, delta);
        if (_sceneCamera)
        {
            const auto& pos = _sceneCamera->GetPosition();
            const auto& rot = _sceneCamera->GetRotation();
            const auto& forward = _sceneCamera->GetForward();
            ImGui::Text("Camera Pos: X: %.3f / Y: %.3f / Z: %.3f", pos.x, pos.y, pos.z);
            ImGui::Text("Camera Rot: X: %.3f / Y: %.3f / Z: %.3f", rot.x, rot.y, rot.z);
            ImGui::Text("Camera Forward: X: %.3f / Y: %.3f / Z: %.3f", forward.x, forward.y, forward.z);
        }

        _lastFrameTime = elapsedTime;
    }
    ImGui::End();
}

HS_NS_EDITOR_END