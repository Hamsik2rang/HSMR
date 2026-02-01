//
//  GizmoController.cpp
//  HSMR
//
//  ImGuizmo wrapper for 3D object manipulation
//
#include "GizmoController.h"
#include "Camera.h"
#include "SceneObject.h"

#include "imgui.h"

// ImGuizmo - HAS_IMGUIZMO is defined by CMake when ImGuizmo is available
#ifdef HAS_IMGUIZMO
#include "ImGuizmo.h"
#endif

HS_NS_BEGIN

void GizmoController::ProcessInput()
{
    ImGuiIO& io = ImGui::GetIO();

    // Skip if ImGui wants keyboard input
    if (io.WantCaptureKeyboard) return;

    // Mode shortcuts (only when not using gizmo)
    if (!_isUsing)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_W))
        {
            _mode = Mode::Translate;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_E))
        {
            _mode = Mode::Rotate;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_R))
        {
            _mode = Mode::Scale;
        }

        // Toggle local/world space
        if (ImGui::IsKeyPressed(ImGuiKey_L))
        {
            _space = (_space == Space::Local) ? Space::World : Space::Local;
        }

        // Toggle snap
        if (ImGui::IsKeyPressed(ImGuiKey_X))
        {
            _snapEnabled = !_snapEnabled;
        }
    }
}

bool GizmoController::Manipulate(Camera* camera, SceneObject* object)
{
    if (!_enabled || !camera || !object) return false;

#if HAS_IMGUIZMO
    ImGuizmo::SetOrthographic(camera->GetProjectionType() == Camera::EProjectionType::ORTHOGRAPHIC);
    ImGuizmo::SetDrawlist();

    // Set viewport
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    // Get matrices
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 proj = camera->GetProjectionMatrix();
    glm::mat4 model = object->GetWorldMatrix();

    // Convert mode
    ImGuizmo::OPERATION operation;
    switch (_mode)
    {
    case Mode::Translate:
        operation = ImGuizmo::TRANSLATE;
        break;
    case Mode::Rotate:
        operation = ImGuizmo::ROTATE;
        break;
    case Mode::Scale:
        operation = ImGuizmo::SCALE;
        break;
    default:
        operation = ImGuizmo::TRANSLATE;
        break;
    }

    // Convert space
    ImGuizmo::MODE mode = (_space == Space::Local) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

    // Snap values
    float* snapValues = nullptr;
    float translateSnapVec[3] = { _translateSnap, _translateSnap, _translateSnap };
    float rotateSnapVec[1] = { _rotateSnap };
    float scaleSnapVec[1] = { _scaleSnap };

    if (_snapEnabled)
    {
        switch (_mode)
        {
        case Mode::Translate:
            snapValues = translateSnapVec;
            break;
        case Mode::Rotate:
            snapValues = rotateSnapVec;
            break;
        case Mode::Scale:
            snapValues = scaleSnapVec;
            break;
        }
    }

    // Store previous matrix for delta calculation
    glm::mat4 previousModel = model;

    // Manipulate
    bool changed = ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(proj),
        operation,
        mode,
        glm::value_ptr(model),
        glm::value_ptr(_deltaMatrix),
        snapValues
    );

    // Update status
    _isUsing = ImGuizmo::IsUsing();
    _isOver = ImGuizmo::IsOver();

    if (changed)
    {
        // Decompose matrix and apply to object
        glm::vec3 pos, rot, scale;
        float matrixData[16];
        memcpy(matrixData, glm::value_ptr(model), sizeof(float) * 16);

        ImGuizmo::DecomposeMatrixToComponents(
            matrixData,
            glm::value_ptr(pos),
            glm::value_ptr(rot),
            glm::value_ptr(scale)
        );

        // Apply transform (rotation is in degrees from ImGuizmo)
        object->SetPosition(pos);
        object->SetRotation(glm::radians(rot));
        object->SetScale(scale);
    }

    return changed;
#else
    // Fallback without ImGuizmo - show simple UI controls
    ImGuiIO& io = ImGui::GetIO();

    // Draw a simple axis indicator at object position
    glm::vec2 screenPos = camera->WorldToScreenPoint(object->GetPosition());

    // Convert from NDC to screen coordinates
    screenPos.x = (screenPos.x + 1.0f) * 0.5f * io.DisplaySize.x;
    screenPos.y = (1.0f - screenPos.y) * 0.5f * io.DisplaySize.y;

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Draw axis lines
    float axisLength = 50.0f;
    drawList->AddLine(
        ImVec2(screenPos.x, screenPos.y),
        ImVec2(screenPos.x + axisLength, screenPos.y),
        IM_COL32(255, 0, 0, 255), 2.0f); // X axis - red

    drawList->AddLine(
        ImVec2(screenPos.x, screenPos.y),
        ImVec2(screenPos.x, screenPos.y - axisLength),
        IM_COL32(0, 255, 0, 255), 2.0f); // Y axis - green

    // Show transform controls in Inspector instead
    return false;
#endif
}

void GizmoController::DrawGrid(Camera* camera, float gridSize)
{
    if (!camera) return;

#if HAS_IMGUIZMO
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 proj = camera->GetProjectionMatrix();
    glm::mat4 identity = glm::mat4(1.0f);

    ImGuizmo::DrawGrid(
        glm::value_ptr(view),
        glm::value_ptr(proj),
        glm::value_ptr(identity),
        gridSize
    );
#else
    // Fallback grid drawing with ImGui
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Draw simple grid lines on XZ plane
    int gridLines = static_cast<int>(gridSize);
    float halfSize = gridSize * 0.5f;

    for (int i = -gridLines; i <= gridLines; ++i)
    {
        glm::vec3 startX(-halfSize, 0.0f, static_cast<float>(i));
        glm::vec3 endX(halfSize, 0.0f, static_cast<float>(i));

        glm::vec2 screenStart = camera->WorldToScreenPoint(startX);
        glm::vec2 screenEnd = camera->WorldToScreenPoint(endX);

        // Convert NDC to screen
        screenStart.x = (screenStart.x + 1.0f) * 0.5f * io.DisplaySize.x;
        screenStart.y = (1.0f - screenStart.y) * 0.5f * io.DisplaySize.y;
        screenEnd.x = (screenEnd.x + 1.0f) * 0.5f * io.DisplaySize.x;
        screenEnd.y = (1.0f - screenEnd.y) * 0.5f * io.DisplaySize.y;

        ImU32 color = (i == 0) ? IM_COL32(255, 0, 0, 128) : IM_COL32(128, 128, 128, 64);
        drawList->AddLine(ImVec2(screenStart.x, screenStart.y), ImVec2(screenEnd.x, screenEnd.y), color);

        // Z lines
        glm::vec3 startZ(static_cast<float>(i), 0.0f, -halfSize);
        glm::vec3 endZ(static_cast<float>(i), 0.0f, halfSize);

        screenStart = camera->WorldToScreenPoint(startZ);
        screenEnd = camera->WorldToScreenPoint(endZ);

        screenStart.x = (screenStart.x + 1.0f) * 0.5f * io.DisplaySize.x;
        screenStart.y = (1.0f - screenStart.y) * 0.5f * io.DisplaySize.y;
        screenEnd.x = (screenEnd.x + 1.0f) * 0.5f * io.DisplaySize.x;
        screenEnd.y = (1.0f - screenEnd.y) * 0.5f * io.DisplaySize.y;

        color = (i == 0) ? IM_COL32(0, 0, 255, 128) : IM_COL32(128, 128, 128, 64);
        drawList->AddLine(ImVec2(screenStart.x, screenStart.y), ImVec2(screenEnd.x, screenEnd.y), color);
    }
#endif
}

void GizmoController::DrawViewManipulator(Camera* camera, float size)
{
    if (!camera) return;

#if HAS_IMGUIZMO
    ImGuiIO& io = ImGui::GetIO();

    // Position in top-right corner
    float x = io.DisplaySize.x - size - 10.0f;
    float y = 10.0f;

    glm::mat4 view = camera->GetViewMatrix();
    float distance = glm::length(camera->GetPosition());

    ImGuizmo::ViewManipulate(
        glm::value_ptr(view),
        distance,
        ImVec2(x, y),
        ImVec2(size, size),
        0x10101010
    );

    // Note: To apply view changes, would need to decompose view matrix
    // back to camera position/rotation
#endif
}

HS_NS_END
