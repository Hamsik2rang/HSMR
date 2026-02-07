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

    drawViewGizmo();

    ImGui::End();

    ImGui::PopStyleVar();
}

void ScenePanel::drawViewGizmo()
{
    if (!_camera) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Gizmo center: top-right corner
    float halfSize = _viewGizmoSize * 0.5f;
    ImVec2 center(
        windowPos.x + windowSize.x - halfSize - _viewGizmoMargin,
        windowPos.y + halfSize + (_viewGizmoMargin * 4.0f)
    );

    float axisLength  = halfSize;
    float coneHeight  = 14.0f;
    float coneRadius  = 6.0f;

    // Background
    drawList->AddCircleFilled(center, halfSize, IM_COL32(20, 20, 20, 140), 32);
    drawList->AddCircle(center, halfSize, IM_COL32(80, 80, 80, 180), 32, 1.0f);

    // View rotation (world -> view upper 3x3)
    glm::mat3 viewRot(_camera->GetViewMatrix());

    struct Axis
    {
        glm::vec3 worldDir;
        ImU32     color;
        float     sx, sy, depth;
    };

    Axis axes[3] = {
        { {1, 0, 0}, IM_COL32(220, 60, 60, 255),  0, 0, 0 },
        { {0, 1, 0}, IM_COL32(60, 190, 60, 255),   0, 0, 0 },
        { {0, 0, 1}, IM_COL32(80, 130, 230, 255),  0, 0, 0 },
    };

    // Project each axis through view rotation
    for (auto& a : axes)
    {
        glm::vec3 v = viewRot * a.worldDir;
        a.sx    = v.x;
        a.sy    = -v.y; // screen Y flipped
        a.depth = v.z;  // +z = toward camera
    }

    // Sort back-to-front (ascending depth)
    int order[3] = { 0, 1, 2 };
    for (int i = 0; i < 2; i++)
        for (int j = i + 1; j < 3; j++)
            if (axes[order[i]].depth > axes[order[j]].depth)
            {
                int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
            }

    bool windowHovered = ImGui::IsWindowHovered();
    ImGuiIO& io = ImGui::GetIO();

    for (int idx = 0; idx < 3; idx++)
    {
        Axis& a = axes[order[idx]];

        // Dim axes pointing away from camera
        float alpha = (a.depth < 0.0f) ? 0.35f : 1.0f;
        uint8_t cr = (a.color >> IM_COL32_R_SHIFT) & 0xFF;
        uint8_t cg = (a.color >> IM_COL32_G_SHIFT) & 0xFF;
        uint8_t cb = (a.color >> IM_COL32_B_SHIFT) & 0xFF;
        ImU32 col = IM_COL32(cr, cg, cb, static_cast<uint8_t>(255 * alpha));

        ImVec2 tip(center.x + a.sx * axisLength, center.y + a.sy * axisLength);

        float len = sqrtf(a.sx * a.sx + a.sy * a.sy);
        if (len > 0.001f)
        {
            float dx = a.sx / len;
            float dy = a.sy / len;
            float px = -dy, py = dx; // perpendicular

            // Cone base
            ImVec2 coneBase(tip.x - dx * coneHeight, tip.y - dy * coneHeight);

            // Shaft line: center to cone base
            drawList->AddLine(center, coneBase, col, 3.0f);

            // Cone body: filled triangle
            ImVec2 p1(tip.x, tip.y);
            ImVec2 p2(coneBase.x + px * coneRadius, coneBase.y + py * coneRadius);
            ImVec2 p3(coneBase.x - px * coneRadius, coneBase.y - py * coneRadius);
            drawList->AddTriangleFilled(p1, p2, p3, col);
        }
        else
        {
            // Axis pointing directly at/away from camera: draw a dot
            drawList->AddCircleFilled(center, coneRadius, col, 12);
        }

        // Click cone -> snap camera to that axis view
        if (windowHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            float mx = io.MousePos.x - tip.x;
            float my = io.MousePos.y - tip.y;
            if (mx * mx + my * my < 14.0f * 14.0f)
            {
                float camDist = glm::length(_camera->GetPosition());
                if (camDist < 0.1f) camDist = 5.0f;

                glm::vec3 newPos  = a.worldDir * camDist;
                glm::vec3 forward = -a.worldDir;

                float pitch = asinf(forward.y);
                float yaw   = atan2f(forward.x, forward.z);

                _camera->SetPosition(newPos);
                _camera->SetRotation(glm::vec3(pitch, yaw, 0.0f));
                _camera->Update();
            }
        }
    }
}

HS_NS_EDITOR_END
