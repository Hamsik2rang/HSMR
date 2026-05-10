#include "Editor/Panel/SimpleInspectorPanel.h"
#include "Editor/Panel/EditorPanelFrame.h"
#include "Editor/GUI/EditorFeedbackWidgets.h"

#include "ThirdParty/ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

bool SimpleInspectorPanel::Setup()
{
    return true;
}

void SimpleInspectorPanel::Cleanup()
{
    _mainLightEntity  = Entity::Invalid();
    _mainCameraEntity = Entity::Invalid();
    _targetEntity     = Entity::Invalid();
}

void SimpleInspectorPanel::Draw()
{
    if (!IsVisible())
    {
        return;
    }

    EditorPanelWindowOptions panelOptions{};
    panelOptions.pOpen = GetVisibilityBinding();
    EditorPanelFrame::BeginStandardPanel("Control Panel", panelOptions);

    if (_mainCameraEntity.IsValid())
    {
        if (ImGui::CollapsingHeader("Camera"))
        {
            auto& transform = _mainCameraEntity.GetComponent<TransformComponent>();
            auto worldPos   = transform.GetWorldPosition();
            drawVec3Control("Camera Position", worldPos, 0.0f, 0.1f);
            auto worldRot = transform.GetWorldEulerAngles();
            drawVec3Control("Camera Rotation", worldRot, 0.0f, 1.0f);
        }
    }

    if (_mainLightEntity.IsValid() && _mainLightEntity.HasComponent<LightComponent>())
    {
        if (ImGui::CollapsingHeader("Main Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto& transform = _mainLightEntity.GetComponent<TransformComponent>();
            auto& light     = _mainLightEntity.GetComponent<LightComponent>();

            auto worldPos = transform.GetWorldPosition();
            if (drawVec3Control("Position", worldPos, 0.0f, 0.1f))
            {
                transform.SetWorldPosition(worldPos);
            }

            // Angle-based rotation. The renderer derives light direction from
            // transform.forward when LightComponent.direction stays at zero,
            // so editing rotation here directly steers the light.
            auto worldRot = transform.GetWorldEulerAngles();
            if (drawVec3Control("Rotation", worldRot, 0.0f, 1.0f))
            {
                transform.SetWorldEulerAngles(worldRot);
            }

            ImGui::SliderFloat("Intensity", &light.intensity, 0.0f, 20.0f);
            ImGui::ColorEdit3("Color", &light.color.x);
            ImGui::Checkbox("Enabled", &light.isEnabled);

            if (ImGui::Button("Reset (sun-ish)"))
            {
                transform.SetWorldEulerAngles(glm::vec3(-60.0f, 30.0f, 0.0f));
                light.intensity = 5.0f;
                light.color     = glm::vec3(1.0f);
                light.isEnabled = true;
            }
        }
    }

    if (_targetEntity.IsValid())
    {
        auto& transform = _targetEntity.GetComponent<TransformComponent>();

        if (ImGui::CollapsingHeader("Target Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto pos = transform.position;
            if (drawVec3Control("Position", pos, 0.0f, 0.1f))
            {
                transform.SetPosition(pos);
            }
            auto euler = transform.GetEulerAngles();
            if (drawVec3Control("Rotation", euler, 0.0f, 1.0f))
            {
                transform.SetEulerAngles(euler);
            }
            auto scl = transform.scale;
            if (drawVec3Control("Scale", scl, 1.0f, 0.1f))
            {
                transform.SetScale(scl);
            }
        }
    }
    else
    {
        EditorFeedbackWidgets::EmptyState("Entity is not selected");
    }

    EditorPanelFrame::EndStandardPanel();
}

HS_NS_EDITOR_END
