#include "Editor/Panel/SimpleInspectorPanel.h"

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
    static bool showDemo = true;
    if (showDemo)
    {
        ImGui::ShowDemoWindow(&showDemo);
    }
    ImGui::Begin("Control Panel");
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

    if (_mainCameraEntity.IsValid())
    {
        if (ImGui::CollapsingHeader("Camera"))
        {
            auto& transform = _mainCameraEntity.GetComponent<TransformComponent>();
            // Camera는 GUI 조작으로 이동시키지 않아야 함.
            auto worldPos   = transform.GetWorldPosition();
            drawVec3Control("Camera Position", worldPos, 0.0f, 0.1f);
            auto worldRot = transform.GetWorldEulerAngles();
            drawVec3Control("Camera Rotation", worldRot, 0.0f, 1.0f);
        }
    }

    if (_mainLightEntity.IsValid())
    {
        if (ImGui::CollapsingHeader("Main Light"))
        {
            auto& transform = _mainLightEntity.GetComponent<TransformComponent>();
            auto worldPos   = transform.GetWorldPosition();
            drawVec3Control("Light Position", worldPos, 0.0f, 0.1f);
            transform.SetWorldPosition(worldPos);
            
            auto worldRot = transform.GetWorldEulerAngles();
            drawVec3Control("Light Rotation", worldRot, 0.0f, 1.0f);
            transform.SetWorldEulerAngles(worldRot);
        }
    }

    if (_targetEntity.IsValid())
    {
    }
    else
    {
        ImGui::TextDisabled("Entity is not selected");
    }

    ImGui::End();
}

HS_NS_EDITOR_END
