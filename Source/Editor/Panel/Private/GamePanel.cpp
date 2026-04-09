#include "Editor/Panel/GamePanel.h"

#include "Editor/Core/EditorContext.h"
#include "Editor/GUI/EditorIcons.h"
#include "Editor/GUI/ImGuiExtension.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "ImGui/imgui.h"

#include <vector>

HS_NS_EDITOR_BEGIN

namespace
{
std::vector<Entity> collectSceneCameras(Scene* scene)
{
    std::vector<Entity> cameras;
    if (!scene)
    {
        return cameras;
    }

    auto view = scene->GetRegistry().view<CameraComponent>();
    for (auto [entity, camera] : view.each())
    {
        cameras.emplace_back(entity, scene);
    }

    std::stable_sort(
        cameras.begin(),
        cameras.end(),
        [](const Entity& lhs, const Entity& rhs) -> bool
        {
            const CameraComponent& lhsCamera = lhs.GetComponent<CameraComponent>();
            const CameraComponent& rhsCamera = rhs.GetComponent<CameraComponent>();

            if (lhsCamera.isActive != rhsCamera.isActive)
            {
                return lhsCamera.isActive && !rhsCamera.isActive;
            }

            if (lhsCamera.priority != rhsCamera.priority)
            {
                return lhsCamera.priority > rhsCamera.priority;
            }

            if (lhsCamera.isPrimary != rhsCamera.isPrimary)
            {
                return lhsCamera.isPrimary && !rhsCamera.isPrimary;
            }

            return entt::to_integral(lhs.GetHandle()) < entt::to_integral(rhs.GetHandle());
        });

    return cameras;
}
}

GamePanel::GamePanel(Window* window)
    : Panel(window)
    , _resolution(800, 600)
{
}

GamePanel::~GamePanel()
{
}

bool GamePanel::Setup()
{
    return true;
}

void GamePanel::Cleanup()
{
}

Entity GamePanel::ResolveCamera(Scene* scene) const
{
    if (!scene)
    {
        return Entity();
    }

    if (!_useAutoCamera &&
        _selectedCamera.IsValid() &&
        _selectedCamera.GetScene() == scene &&
        _selectedCamera.HasComponent<CameraComponent>())
    {
        return _selectedCamera;
    }

    return scene->GetBestGameCamera();
}

std::string GamePanel::getCameraLabel(Entity camera) const
{
    if (!camera.IsValid() || !camera.HasComponent<TagComponent>())
    {
        return "No Camera";
    }

    std::string label = camera.GetComponent<TagComponent>().name;
    if (camera.HasComponent<CameraComponent>())
    {
        const CameraComponent& cameraComponent = camera.GetComponent<CameraComponent>();
        label += " [P=" + std::to_string(cameraComponent.priority) + "]";
    }

    return label;
}

void GamePanel::drawMenuBar(Scene* scene)
{
    if (!ImGui::BeginMenuBar())
    {
        return;
    }

    ImGui::TextUnformatted("Game");
    ImGui::SameLine();

    bool useAutoCamera = _useAutoCamera;
    if (ImGui::Checkbox("Auto", &useAutoCamera))
    {
        _useAutoCamera = useAutoCamera;
        if (_useAutoCamera)
        {
            _selectedCamera = Entity();
        }
    }

    ImGui::SameLine();

    Entity resolvedCamera = ResolveCamera(scene);
    std::string previewLabel = _useAutoCamera
        ? ("Auto: " + getCameraLabel(resolvedCamera))
        : getCameraLabel(_selectedCamera);

    ImGui::SetNextItemWidth(220.0f);
    if (ImGui::BeginCombo("##GameCamera", previewLabel.c_str()))
    {
        if (scene)
        {
            std::vector<Entity> cameras = collectSceneCameras(scene);
            for (const Entity& camera : cameras)
            {
                bool isSelected = !_useAutoCamera && camera == _selectedCamera;
                std::string label = getCameraLabel(camera);

                if (camera.HasComponent<CameraComponent>() && !camera.GetComponent<CameraComponent>().isActive)
                {
                    label += " (Inactive)";
                }

                if (ImGui::Selectable(label.c_str(), isSelected))
                {
                    _selectedCamera = camera;
                    _useAutoCamera = false;
                }

                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }

        ImGui::EndCombo();
    }

    ImGui::EndMenuBar();
}

void GamePanel::Draw()
{
    auto& vis = EditorContext::Get().GetPanelVisibility();
    if (!vis.game)
    {
        return;
    }

    ImGui::Begin(
        "Game",
        &vis.game,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_MenuBar);

    Scene* scene = EditorContext::Get().GetActiveScene();
    drawMenuBar(scene);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::BeginChild("GameViewport", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    _resolution.width = static_cast<uint32>(availableSize.x > 1.0f ? availableSize.x : 1.0f);
    _resolution.height = static_cast<uint32>(availableSize.y > 1.0f ? availableSize.y : 1.0f);

    if (_currentRenderTarget)
    {
        ImVec2 viewportSize(
            static_cast<float>(_currentRenderTarget->GetWidth()),
            static_cast<float>(_currentRenderTarget->GetHeight()));
        ImGuiExtension::ImageOffscreen(_currentRenderTarget->GetColorTexture(0), viewportSize);
    }
    else
    {
        ImGui::Dummy(availableSize);
    }

    if (!ResolveCamera(scene).IsValid())
    {
        ImVec2 overlayPos = ImGui::GetWindowPos();
        overlayPos.x += 12.0f;
        overlayPos.y += ImGui::GetFrameHeight() + 12.0f;
        ImGui::SetCursorScreenPos(overlayPos);
        ImGui::TextDisabled("%s No active scene camera", EditorIcons::Camera);
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::End();
}

HS_NS_EDITOR_END
