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
            auto worldPos = transform.GetWorldPosition();
            drawVec3Control("Position", worldPos, 0.0f, 0.1f);
            
            
        }
    }

    if (_mainLightEntity.IsValid())
    {
        bool open = ImGui::CollapsingHeader("Main Light");
        {
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
