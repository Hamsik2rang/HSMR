#include "Editor/Panel/ScenePanel.h"

#include "Core/HAL/Input.h"
#include "RHI/ResourceHandle.h"
#include "Editor/GUI/ImGuiExtension.h"

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

    ImGui::End();

    ImGui::PopStyleVar();
}

HS_NS_EDITOR_END
