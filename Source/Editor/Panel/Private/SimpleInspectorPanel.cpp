#include "Editor/Panel/SimpleInspectorPanel.h"

#include "ThirdParty/ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "Editor/Core/EditorCamera.h"

#include "Core/HAL/Input.h"

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
    _editorCamera     = nullptr;
}

void SimpleInspectorPanel::Draw()
{
    ImGui::Begin("Control Panel");

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

    // Gizmo controls
    if (ImGui::CollapsingHeader("Gizmo", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // W/E/R keyboard shortcuts (only when no text input is active)
        if (!ImGui::GetIO().WantTextInput && !Input::IsPressed(Input::Button::MouseRight))
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Q))
            {
                _gizmoOperation = ImGuizmo::TRANSLATE;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_W))
            {
                _gizmoOperation = ImGuizmo::ROTATE;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_E))
            {
                _gizmoOperation = ImGuizmo::SCALE;
            }
        }

        // Operation radio buttons
        if (ImGui::RadioButton("Translate", _gizmoOperation == ImGuizmo::TRANSLATE))
        {
            _gizmoOperation = ImGuizmo::TRANSLATE;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", _gizmoOperation == ImGuizmo::ROTATE))
        {
            _gizmoOperation = ImGuizmo::ROTATE;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", _gizmoOperation == ImGuizmo::SCALE))
        {
            _gizmoOperation = ImGuizmo::SCALE;
        }

        // Local/World mode toggle (not applicable for Scale)
        if (_gizmoOperation != ImGuizmo::SCALE)
        {
            if (ImGui::RadioButton("Local", _gizmoMode == ImGuizmo::LOCAL))
            {
                _gizmoMode = ImGuizmo::LOCAL;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("World", _gizmoMode == ImGuizmo::WORLD))
            {
                _gizmoMode = ImGuizmo::WORLD;
            }
        }

        // Snap controls
        ImGui::Checkbox("Snap", &_useSnap);
        ImGui::SameLine();
        if (_gizmoOperation == ImGuizmo::TRANSLATE)
        {
            ImGui::InputFloat3("Snap Value", glm::value_ptr(_snapValue));
        }
        else if (_gizmoOperation == ImGuizmo::ROTATE)
        {
            ImGui::InputFloat("Angle Snap", &_snapValue.x);
        }
        else if (_gizmoOperation == ImGuizmo::SCALE)
        {
            ImGui::InputFloat("Scale Snap", &_snapValue.x);
        }
    }

    if (_targetEntity.IsValid())
    {
        auto& transform = _targetEntity.GetComponent<TransformComponent>();

        // Display current transform values
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

        // Render ImGuizmo
        if (_editorCamera)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

            // Tell ImGuizmo to accept mouse input from the SceneViewport window,
            // since the gizmo is drawn on the foreground drawlist but the mouse
            // hovers over the scene viewport, not the Control Panel.
            ImGuiWindow* sceneWindow = ImGui::FindWindowByName("##SceneViewport");
            if (sceneWindow)
            {
                ImGuizmo::SetAlternativeWindow(sceneWindow);
            }

            ImGuiViewport* mainViewport = ImGui::GetMainViewport();
            ImGuizmo::SetRect(
                mainViewport->Pos.x, mainViewport->Pos.y,
                mainViewport->Size.x, mainViewport->Size.y
            );

            // ImGuizmo extracts camera direction from inverse(view) column 2.
            // In LH that column points forward, but ImGuizmo expects RH where
            // it points backward.  Negate only the Z row of the LH view matrix
            // and pair with perspectiveRH so screen x,y stay identical to LH
            // while the camera-direction extraction matches RH convention.
            glm::mat4 viewMatrix = _editorCamera->GetViewMatrix();
            viewMatrix[0][2]     = -viewMatrix[0][2];
            viewMatrix[1][2]     = -viewMatrix[1][2];
            viewMatrix[2][2]     = -viewMatrix[2][2];
            viewMatrix[3][2]     = -viewMatrix[3][2];

            glm::mat4 projMatrix = glm::perspectiveRH(
                _editorCamera->GetFov(), _editorCamera->GetAspectRatio(),
                _editorCamera->GetNearZ(), _editorCamera->GetFarZ()
            );

            glm::mat4 objectMatrix = transform.worldMatrix;
            glm::mat4 deltaMatrix(1.0f);

            float* snap = _useSnap ? glm::value_ptr(_snapValue) : nullptr;

            bool manipulated = ImGuizmo::Manipulate(
                glm::value_ptr(viewMatrix),
                glm::value_ptr(projMatrix),
                _gizmoOperation,
                _gizmoMode,
                glm::value_ptr(objectMatrix),
                glm::value_ptr(deltaMatrix),
                snap
            );

            if (manipulated)
            {
                if (!transform.HasParent())
                {
                    // No parent: world == local, decompose and apply directly
                    float matTranslation[3], matRotation[3], matScale[3];
                    ImGuizmo::DecomposeMatrixToComponents(
                        glm::value_ptr(objectMatrix),
                        matTranslation, matRotation, matScale
                    );
                    transform.SetPosition(glm::vec3(matTranslation[0], matTranslation[1], matTranslation[2]));
                    transform.SetEulerAngles(glm::vec3(matRotation[0], matRotation[1], matRotation[2]));
                    transform.SetScale(glm::vec3(matScale[0], matScale[1], matScale[2]));
                }
                else
                {
                    // Has parent: objectMatrix is the new world matrix from ImGuizmo
                    // Convert back to local space: newLocal = inv(parentWorld) * newWorld
                    glm::mat4 localMatrix = transform.GetLocalMatrix();
                    glm::mat4 parentWorld = transform.worldMatrix * glm::inverse(localMatrix);
                    glm::mat4 newLocal    = glm::inverse(parentWorld) * objectMatrix;

                    float matTranslation[3], matRotation[3], matScale[3];
                    ImGuizmo::DecomposeMatrixToComponents(
                        glm::value_ptr(newLocal),
                        matTranslation, matRotation, matScale
                    );
                    transform.SetPosition(glm::vec3(matTranslation[0], matTranslation[1], matTranslation[2]));
                    transform.SetEulerAngles(glm::vec3(matRotation[0], matRotation[1], matRotation[2]));
                    transform.SetScale(glm::vec3(matScale[0], matScale[1], matScale[2]));
                }
            }
        }
    }
    else
    {
        ImGui::TextDisabled("Entity is not selected");
    }

    ImGui::End();
}

HS_NS_EDITOR_END
