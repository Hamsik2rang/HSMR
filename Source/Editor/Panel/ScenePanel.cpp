#include "Editor/Panel/ScenePanel.h"

#include "Core/HAL/Input.h"
#include "RHI/ResourceHandle.h"
#include "Editor/GUI/ImGuiExtension.h"

#if HAS_IMGUIZMO
#include "ImGuizmo.h"
#endif

HS_NS_EDITOR_BEGIN

bool ScenePanel::Setup()
{
    _camera = MakeScoped<Camera>();
    _camera->SetAspectRatio(static_cast<float>(_resolution.width) / static_cast<float>(_resolution.height));

    return true;
}

void ScenePanel::Cleanup()
{
}

void ScenePanel::Update(float deltaTime)
{
    if (!Input::IsPressed(Input::Button::MOUSE_RIGHT))
    {
        _isMouseTracking = false;
        return;
    }

    // --- Mouse look ---
    uint16 mouseX, mouseY;
    Input::GetMousePosition(mouseX, mouseY);

    if (_isMouseTracking)
    {
        float dx = static_cast<float>(mouseX) - static_cast<float>(_lastMouseX);
        float dy = static_cast<float>(mouseY) - static_cast<float>(_lastMouseY);

        if (dx != 0.0f || dy != 0.0f)
        {
            float rotateSpeed = _camera->GetRotateSpeed();
            _camera->Rotate(glm::vec3(-dy * rotateSpeed, -dx * rotateSpeed, 0.0f));
        }
    }

    _lastMouseX = mouseX;
    _lastMouseY = mouseY;
    _isMouseTracking = true;

    // --- Keyboard movement ---
    int front = 0, right = 0, up = 0;
    if (Input::IsPressed(Input::Button::W)) front++;
    if (Input::IsPressed(Input::Button::S)) front--;
    if (Input::IsPressed(Input::Button::D)) right++;
    if (Input::IsPressed(Input::Button::A)) right--;
    if (Input::IsPressed(Input::Button::E)) up++;
    if (Input::IsPressed(Input::Button::Q)) up--;

    if (front != 0 || right != 0 || up != 0)
    {
        glm::vec3 moveDir = _camera->GetForward() * static_cast<float>(front)
                          + _camera->GetRight()   * static_cast<float>(right)
                          + glm::vec3(0.0f, 1.0f, 0.0f) * static_cast<float>(up);
        _camera->Move(moveDir * deltaTime * _camera->GetMoveSpeed());
    }

    _camera->Update();
}

void ScenePanel::Draw()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);

    ImGui::SetScrollY(0.0f);
    uint32 width  = _currentRenderTarget->GetWidth();
    uint32 height = _currentRenderTarget->GetHeight();

    ImVec2 viewportSize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    RHITexture* texture = _currentRenderTarget->GetColorTexture(0);

    ImGuiExtension::ImageOffscreen(texture, viewportSize);

    ImVec2 curPanelSize = ImGui::GetWindowSize();
    _resolution.width   = static_cast<uint32>(curPanelSize.x);
    _resolution.height  = static_cast<uint32>(curPanelSize.y);

    if (_camera && _resolution.height > 0)
    {
        _camera->SetAspectRatio(static_cast<float>(_resolution.width) / static_cast<float>(_resolution.height));
    }

    drawViewGizmo();

    ImGui::End();

    ImGui::PopStyleVar();
}

void ScenePanel::drawViewGizmo()
{
#if HAS_IMGUIZMO
    if (!_camera) return;

    // Bind ImGuizmo to current ImGui window's draw list
    ImGuizmo::SetDrawlist();

    // Set ImGuizmo rect to ScenePanel window area
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

    // Top-right position
    float x = windowPos.x + windowSize.x - _viewGizmoSize - _viewGizmoMargin;
    float y = windowPos.y + _viewGizmoMargin;

    // Copy camera view matrix
    glm::mat4 view = _camera->GetViewMatrix();
    float camDistance = glm::length(_camera->GetPosition());

    // ViewManipulate modifies view matrix in-place
    ImGuizmo::ViewManipulate(
        glm::value_ptr(view),
        camDistance,
        ImVec2(x, y),
        ImVec2(_viewGizmoSize, _viewGizmoSize),
        0x10101010
    );

    // Apply changes back to camera when gizmo is being manipulated
    if (ImGuizmo::IsUsingViewManipulate())
    {
        glm::mat4 invView = glm::inverse(view);
        glm::vec3 newPosition = glm::vec3(invView[3]);
        glm::vec3 forward = glm::normalize(-glm::vec3(invView[2]));

        // Reverse euler angles from forward vector
        // Camera convention: front.x = cos(pitch)*sin(yaw), front.y = sin(pitch), front.z = cos(pitch)*cos(yaw)
        float pitch = asin(forward.y);
        float yaw   = atan2(forward.x, forward.z);

        _camera->SetPosition(newPosition);
        _camera->SetRotation(glm::vec3(pitch, yaw, 0.0f));
        _camera->Update();
    }
#endif
}

HS_NS_EDITOR_END
