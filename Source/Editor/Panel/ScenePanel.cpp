//
//  ScenePanel.cpp
//  Editor
//

#include "Editor/Panel/ScenePanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Asset/AssetDatabase.h"

#include "Core/HAL/Input.h"
#include "RHI/ResourceHandle.h"
#include "Editor/GUI/ImGuiExtension.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "Resource/Model.h"

// For matrix decomposition
#define GLM_ENABLE_EXPERIMENTAL
#include "Core/ThirdParty/glm/gtx/matrix_decompose.hpp"

#ifdef HAS_IMGUIZMO
#include <ImGuizmo.h>
#endif

HS_NS_EDITOR_BEGIN

bool ScenePanel::Setup()
{
    _editorCamera = MakeScoped<EditorCamera>();
    _editorCamera->SetAspectRatio(static_cast<float>(_resolution.width) / static_cast<float>(_resolution.height));

    return true;
}

void ScenePanel::Cleanup()
{
}

void ScenePanel::Update(float deltaTime)
{
    static constexpr float moveSpeedDecelFactor = 0.5f; // Deceleration factor when no input is given
    static float currentCameraSpeed           = 0.0f;
    static glm::vec3 moveDir                  = glm::vec3(0.0f);

    // Only process camera input when viewport is hovered and gizmo is not being used
#ifdef HAS_IMGUIZMO
    if (ImGuizmo::IsUsing())
    {
        _isMouseTracking             = false;
        _rightClickStartedInViewport = false;
        return;
    }
#endif

    // Check if right-click just started (using ImGui to detect click event)
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        // Only start camera control if click started in this viewport
        _rightClickStartedInViewport = _viewportHovered;
    }
    if (!_rightClickStartedInViewport)
    {
        return;
    }

    bool isMoveDirectionUpdated = false;
    if (Input::IsPressed(Input::Button::MouseRight))
    {

        // --- Mouse look ---
        uint16 mouseX, mouseY;
        Input::GetMousePosition(mouseX, mouseY);

        if (_isMouseTracking)
        {
            float dx = static_cast<float>(mouseX) - static_cast<float>(_lastMouseX);
            float dy = static_cast<float>(mouseY) - static_cast<float>(_lastMouseY);

            if (dx != 0.0f || dy != 0.0f)
            {
                float rotateSpeed = _editorCamera->GetRotateSpeed();
                // Rotate(yawDelta, pitchDelta)
                _editorCamera->Rotate(dx * rotateSpeed, -dy * rotateSpeed);
            }
        }

        _lastMouseX      = mouseX;
        _lastMouseY      = mouseY;
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
            moveDir = _editorCamera->GetForward() * static_cast<float>(front) +
                      _editorCamera->GetRight() * static_cast<float>(right) +
                      glm::vec3(0.0f, 1.0f, 0.0f) * static_cast<float>(up);
            isMoveDirectionUpdated = true;
        }
        else
        {
            isMoveDirectionUpdated = false;
        }
    }
    else
    {
        _isMouseTracking   = false;
    }

    if (isMoveDirectionUpdated)
    {
        currentCameraSpeed = _editorCamera->GetMoveSpeed();
    }
    else
    {
        currentCameraSpeed = std::max(currentCameraSpeed - moveSpeedDecelFactor, 0.0f);
    }

    if (Math::EpsilonEqual(currentCameraSpeed, 0.0f))
    {
        return;
    }
    _editorCamera->Move(moveDir * deltaTime * currentCameraSpeed);

    _editorCamera->Update();
}

void ScenePanel::Draw()
{
    auto& vis = EditorContext::Get().GetPanelVisibility();
    if (!vis.scene)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);

    // Store viewport state
    _viewportFocused = ImGui::IsWindowFocused();
    _viewportHovered = ImGui::IsWindowHovered();

    ImGui::SetScrollY(0.0f);
    uint32 width  = _currentRenderTarget->GetWidth();
    uint32 height = _currentRenderTarget->GetHeight();

    ImVec2 viewportSize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    RHITexture* texture = _currentRenderTarget->GetColorTexture(0);

    // Get viewport bounds before drawing image
    ImVec2 viewportPos = ImGui::GetCursorScreenPos();
    _viewportMin       = viewportPos;

    ImGuiExtension::ImageOffscreen(texture, viewportSize);

    ImVec2 curPanelSize = ImGui::GetWindowSize();
    _resolution.width   = static_cast<uint32>(curPanelSize.x);
    _resolution.height  = static_cast<uint32>(curPanelSize.y);

    _viewportMax = ImVec2(_viewportMin.x + static_cast<float>(_resolution.width), _viewportMin.y + static_cast<float>(_resolution.height));

    if (_editorCamera && _resolution.height > 0)
    {
        _editorCamera->SetAspectRatio(static_cast<float>(_resolution.width) / static_cast<float>(_resolution.height));
    }

    // Accept ASSET_MODEL drag & drop onto viewport
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MODEL"))
        {
            std::string assetPath(static_cast<const char*>(payload->Data));

            Scene* scene = EditorContext::Get().GetActiveScene();
            if (scene)
            {
                hs::editor::AssetDatabase& assetDB = hs::editor::AssetDatabase::Get();
                hs::Model* model                   = assetDB.LoadModel(assetPath);
                if (model)
                {
                    // Extract display name from asset path
                    std::string entityName = assetPath;
                    size_t lastSlash       = entityName.rfind('/');
                    if (lastSlash != std::string::npos)
                        entityName = entityName.substr(lastSlash + 1);
                    size_t dot = entityName.rfind('.');
                    if (dot != std::string::npos)
                        entityName = entityName.substr(0, dot);

                    Entity entity      = scene->CreateEntity(entityName);
                    auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
                    meshRenderer.mesh  = model->GetMesh();
                    if (model->GetMaterial())
                    {
                        meshRenderer.materials.push_back(model->GetMaterial());
                    }

                    EditorContext::Get().SetSelectedEntity(entity);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Handle picking before gizmo (so clicking empty space clears selection)
    handlePicking();

    // Draw transform gizmo for selected entity
    drawTransformGizmo();

    // Draw view orientation gizmo
    drawViewGizmo();

    ImGui::End();

    ImGui::PopStyleVar();
}

void ScenePanel::drawTransformGizmo()
{
#ifdef HAS_IMGUIZMO
    Entity selectedEntity = EditorContext::Get().GetSelectedEntity();
    if (!selectedEntity.IsValid() || !selectedEntity.HasComponent<TransformComponent>())
        return;

    if (!_editorCamera)
        return;

    // Get gizmo settings from EditorContext
    auto& context   = EditorContext::Get();
    auto gizmoOp    = context.GetGizmoOperation();
    auto gizmoSpace = context.GetGizmoSpace();

    // Map to ImGuizmo types
    ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
    switch (gizmoOp)
    {
    case EditorContext::GizmoOperation::Translate:
        operation = ImGuizmo::TRANSLATE;
        break;
    case EditorContext::GizmoOperation::Rotate:
        operation = ImGuizmo::ROTATE;
        break;
    case EditorContext::GizmoOperation::Scale:
        operation = ImGuizmo::SCALE;
        break;
    }

    ImGuizmo::MODE mode = (gizmoSpace == EditorContext::GizmoSpace::Local)
                              ? ImGuizmo::LOCAL
                              : ImGuizmo::WORLD;

    // Set up ImGuizmo
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    // Set the gizmo rect to match our viewport
    ImGuizmo::SetRect(_viewportMin.x, _viewportMin.y, static_cast<float>(_resolution.width), static_cast<float>(_resolution.height));

    // Get matrices
    glm::mat4 viewMatrix = _editorCamera->GetViewMatrix();
    glm::mat4 projMatrix = _editorCamera->GetProjectionMatrix();

    // Get entity transform
    auto& transform        = selectedEntity.GetComponent<TransformComponent>();
    glm::mat4 entityMatrix = transform.worldMatrix;

    // Snapping
    glm::vec3 snapValues(1.0f);
    bool useSnap = context.GetUseSnap();
    if (useSnap)
    {
        float snapValue = context.GetSnapValue();
        if (operation == ImGuizmo::ROTATE)
            snapValues = glm::vec3(15.0f); // 15 degree snap for rotation
        else
            snapValues = glm::vec3(snapValue);
    }

    // Manipulate
    glm::mat4 deltaMatrix(1.0f);
    bool manipulated = ImGuizmo::Manipulate(
        glm::value_ptr(viewMatrix),
        glm::value_ptr(projMatrix),
        operation,
        mode,
        glm::value_ptr(entityMatrix),
        glm::value_ptr(deltaMatrix),
        useSnap ? glm::value_ptr(snapValues) : nullptr
    );

    // If gizmo was manipulated, update the transform
    if (manipulated)
    {
        context.SetGizmoActive(true);

        // Decompose the new matrix
        glm::vec3 translation, scale, skew;
        glm::quat rotation;
        glm::vec4 perspective;
        glm::decompose(entityMatrix, scale, rotation, translation, skew, perspective);

        // Handle parent transform if entity has a parent
        if (transform.HasParent())
        {
            // Get parent's world matrix
            Scene* scene        = selectedEntity.GetScene();
            Entity parentEntity = scene->GetEntity(transform.parent);
            if (parentEntity.IsValid() && parentEntity.HasComponent<TransformComponent>())
            {
                const auto& parentTransform = parentEntity.GetComponent<TransformComponent>();
                glm::mat4 parentWorldInv    = glm::inverse(parentTransform.worldMatrix);

                // Convert to local space
                glm::mat4 localMatrix = parentWorldInv * entityMatrix;
                glm::decompose(localMatrix, scale, rotation, translation, skew, perspective);
            }
        }

        // Update transform
        transform.SetPosition(translation);
        transform.SetRotation(rotation);
        transform.SetScale(scale);
    }
    else
    {
        context.SetGizmoActive(ImGuizmo::IsUsing());
    }
#endif
}

void ScenePanel::handlePicking()
{
#ifdef HAS_IMGUIZMO
    // Don't pick while using gizmo
    if (ImGuizmo::IsOver() || ImGuizmo::IsUsing())
        return;
#endif

    // Only pick on left click in viewport
    if (!_viewportHovered || !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;

    // Don't pick if right mouse is held (camera movement)
    if (Input::IsPressed(Input::Button::MouseRight))
        return;

    ImGuiIO& io  = ImGui::GetIO();
    float mouseX = io.MousePos.x;
    float mouseY = io.MousePos.y;

    // Check if mouse is within viewport bounds
    if (mouseX < _viewportMin.x || mouseX > _viewportMax.x ||
        mouseY < _viewportMin.y || mouseY > _viewportMax.y)
        return;

    // Convert to viewport-local coordinates (0-1 range)
    float viewportX = (mouseX - _viewportMin.x) / static_cast<float>(_resolution.width);
    float viewportY = (mouseY - _viewportMin.y) / static_cast<float>(_resolution.height);

    // Generate ray
    glm::vec3 rayDir    = screenToWorldRay(viewportX, viewportY);
    glm::vec3 rayOrigin = _editorCamera->GetPosition();

    // Pick entity
    Entity picked = pickEntity(rayOrigin, rayDir);

    // Update selection
    if (picked.IsValid())
    {
        EditorContext::Get().SetSelectedEntity(picked);
    }
    else
    {
        EditorContext::Get().ClearSelection();
    }
}

glm::vec3 ScenePanel::screenToWorldRay(float viewportX, float viewportY)
{
    if (!_editorCamera)
        return glm::vec3(0.0f, 0.0f, -1.0f);

    // Convert from [0,1] to NDC [-1,1]
    float ndcX = viewportX * 2.0f - 1.0f;
    float ndcY = 1.0f - viewportY * 2.0f; // Flip Y

    // Get inverse matrices
    glm::mat4 invProj = glm::inverse(_editorCamera->GetProjectionMatrix());
    glm::mat4 invView = glm::inverse(_editorCamera->GetViewMatrix());

    // Unproject near and far points
    glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye           = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec4 rayWorld = invView * rayEye;
    glm::vec3 rayDir   = glm::normalize(glm::vec3(rayWorld));

    return rayDir;
}

Entity ScenePanel::pickEntity(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
    Scene* scene = EditorContext::Get().GetActiveScene();
    if (!scene)
        return Entity();

    Entity closestEntity;
    float closestDist = FLT_MAX;

    // First pass: AABB picking for entities with MeshRendererComponent
    auto meshView = scene->View<TransformComponent, MeshRendererComponent>();
    for (auto entityHandle : meshView)
    {
        Entity entity = scene->GetEntity(entityHandle);
        if (!entity.IsValid())
            continue;

        const auto& transform    = entity.GetComponent<TransformComponent>();
        const auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();

        // Use worldBounds if valid, otherwise create default bounds from transform
        AABB bounds = meshRenderer.worldBounds;
        if (!bounds.IsValid())
        {
            // Fallback: Create unit cube bounds at entity position
            glm::vec3 pos        = transform.GetWorldPosition();
            glm::vec3 halfExtent = transform.scale * 0.5f;
            if (glm::length(halfExtent) < 0.25f)
                halfExtent = glm::vec3(0.5f); // Minimum pickable size
            bounds = AABB(pos - halfExtent, pos + halfExtent);
        }

        float t;
        if (bounds.Intersect(rayOrigin, rayDir, t))
        {
            if (t < closestDist)
            {
                closestDist   = t;
                closestEntity = entity;
            }
        }
    }

    // Second pass: Sphere picking for entities without MeshRendererComponent
    auto view = scene->View<TransformComponent, TagComponent>();
    for (auto entityHandle : view)
    {
        Entity entity = scene->GetEntity(entityHandle);
        if (!entity.IsValid())
            continue;

        // Skip if already has MeshRendererComponent (handled above)
        if (entity.HasComponent<MeshRendererComponent>())
            continue;

        const auto& transform = entity.GetComponent<TransformComponent>();
        glm::vec3 entityPos   = transform.GetWorldPosition();

        // Simple sphere test (radius based on scale)
        float radius = glm::length(transform.scale) * 0.5f;
        if (radius < 0.5f) radius = 0.5f; // Minimum pickable size

        // Ray-sphere intersection
        glm::vec3 oc       = rayOrigin - entityPos;
        float a            = glm::dot(rayDir, rayDir);
        float b            = 2.0f * glm::dot(oc, rayDir);
        float c            = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4.0f * a * c;

        if (discriminant >= 0.0f)
        {
            float t = (-b - glm::sqrt(discriminant)) / (2.0f * a);
            if (t > 0.0f && t < closestDist)
            {
                closestDist   = t;
                closestEntity = entity;
            }
        }
    }

    return closestEntity;
}

void ScenePanel::drawViewGizmo()
{
    if (!_editorCamera) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos     = ImGui::GetWindowPos();
    ImVec2 windowSize    = ImGui::GetWindowSize();

    // Gizmo center: top-right corner
    float halfSize = _viewGizmoSize * 0.5f;
    ImVec2 center(
        windowPos.x + windowSize.x - halfSize - _viewGizmoMargin,
        windowPos.y + halfSize + (_viewGizmoMargin * 4.0f)
    );

    float axisLength = halfSize;
    float coneHeight = 14.0f;
    float coneRadius = 6.0f;

    // Background
    drawList->AddCircleFilled(center, halfSize, IM_COL32(20, 20, 20, 140), 32);
    drawList->AddCircle(center, halfSize, IM_COL32(80, 80, 80, 180), 32, 1.0f);

    // View rotation (world -> view upper 3x3)
    glm::mat3 viewRot(_editorCamera->GetViewMatrix());

    struct Axis
    {
        glm::vec3 worldDir;
        ImU32 color;
        float sx, sy, depth;
    };

    Axis axes[3] = {
        {{1, 0, 0}, IM_COL32(250, 60, 60, 255), 0, 0, 0},
        {{0, 1, 0}, IM_COL32(60, 210, 60, 255), 0, 0, 0},
        {{0, 0, 1}, IM_COL32(80, 130, 250, 255), 0, 0, 0},
    };

    // Project each axis through view rotation
    for (auto& a : axes)
    {
        glm::vec3 v = viewRot * a.worldDir;
        a.sx        = v.x;
        a.sy        = -v.y; // screen Y flipped
        a.depth     = -v.z; // -z = Front
    }

    // Sort back-to-front (ascending depth)
    int order[3] = {0, 1, 2};
    for (int i = 0; i < 2; i++)
    {
        for (int j = i + 1; j < 3; j++)
        {
            if (axes[order[i]].depth > axes[order[j]].depth)
            {
                int tmp  = order[i];
                order[i] = order[j];
                order[j] = tmp;
            }
        }
    }

    bool windowHovered = ImGui::IsWindowHovered();
    ImGuiIO& io        = ImGui::GetIO();

    for (int idx = 0; idx < 3; idx++)
    {
        Axis& a = axes[order[idx]];

        // Dim axes pointing away from camera
        float alpha = (a.depth < 0.0f) ? 0.35f : 1.0f;
        uint8_t cr  = (a.color >> IM_COL32_R_SHIFT) & 0xFF;
        uint8_t cg  = (a.color >> IM_COL32_G_SHIFT) & 0xFF;
        uint8_t cb  = (a.color >> IM_COL32_B_SHIFT) & 0xFF;
        ImU32 col   = IM_COL32(cr, cg, cb, static_cast<uint8_t>(255 * alpha));

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
                float camDist = glm::length(_editorCamera->GetPosition());
                if (camDist < 0.1f) camDist = 5.0f;

                glm::vec3 newPos  = a.worldDir * camDist;
                glm::vec3 forward = -a.worldDir;

                float pitch = asinf(forward.y);
                float yaw   = atan2f(forward.x, forward.z);

                _editorCamera->SetPosition(newPos);
                _editorCamera->SetRotation(glm::vec3(pitch, yaw, 0.0f));
                _editorCamera->Update();
            }
        }
    }
}

HS_NS_EDITOR_END
