//
//  InspectorPanel.cpp
//  Editor
//
//  Component editor panel for selected entity
//

#include "Editor/Panel/InspectorPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Asset/AssetDatabase.h"

#include "Scene/Scene.h"
#include "Scene/Components/Components.h"

#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include "ImGui/imgui_internal.h"

#include <cstring>

// Temporary icon definitions if FontAwesome not available
#ifndef ICON_FA_TIMES
#define ICON_FA_TIMES "X"
#endif
#ifndef ICON_FA_CUBE
#define ICON_FA_CUBE "\xef\x86\xb2"
#endif

HS_NS_EDITOR_BEGIN

InspectorPanel::InspectorPanel(Window* window)
    : Panel(window)
{
}

InspectorPanel::~InspectorPanel()
{
}

bool InspectorPanel::Setup()
{
    return true;
}

void InspectorPanel::Cleanup()
{
}

void InspectorPanel::Draw()
{
    ImGui::Begin("Inspector", nullptr);

    Entity selectedEntity = EditorContext::Get().GetSelectedEntity();

    if (!selectedEntity.IsValid())
    {
        ImGui::TextDisabled("No entity selected");
        ImGui::End();
        return;
    }

    // Draw components
    drawTagComponent(selectedEntity);
    drawTransformComponent(selectedEntity);
    drawMeshRendererComponent(selectedEntity);
    drawCameraComponent(selectedEntity);
    drawLightComponent(selectedEntity);

    ImGui::Separator();

    // Add component button
    drawAddComponentButton(selectedEntity);

    ImGui::End();
}

void InspectorPanel::drawTagComponent(Entity entity)
{
    if (!entity.HasComponent<TagComponent>())
        return;

    auto& tag = entity.GetComponent<TagComponent>();

    // Entity name - editable
    char buffer[256];
    strncpy(buffer, tag.name.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##EntityName", buffer, sizeof(buffer)))
    {
        tag.name = buffer;
    }
    ImGui::PopItemWidth();

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Tag", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();

        // Layer
        int layer = static_cast<int>(tag.layer);
        if (ImGui::InputInt("Layer", &layer))
        {
            tag.layer = static_cast<uint32>(glm::max(0, layer));
        }

        // Static flag
        ImGui::Checkbox("Static", &tag.isStatic);

        // Active flag
        ImGui::Checkbox("Active", &tag.isActive);

        ImGui::Unindent();
    }
}

void InspectorPanel::drawTransformComponent(Entity entity)
{
    if (!entity.HasComponent<TransformComponent>())
        return;

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& transform = entity.GetComponent<TransformComponent>();

        ImGui::Indent();

        // Position
        glm::vec3 position = transform.position;
        if (drawVec3Control("Position", position, 0.0f, 0.1f))
        {
            transform.SetPosition(position);
        }

        // Rotation (convert quaternion to euler for editing)
        glm::vec3 eulerDegrees = glm::degrees(glm::eulerAngles(transform.rotation));
        if (drawVec3Control("Rotation", eulerDegrees, 0.0f, 1.0f))
        {
            transform.SetEulerAngles(eulerDegrees);
        }

        // Scale
        glm::vec3 scale = transform.scale;
        if (drawVec3Control("Scale", scale, 1.0f, 0.01f))
        {
            transform.SetScale(scale);
        }

        ImGui::Unindent();
    }
}

void InspectorPanel::drawMeshRendererComponent(Entity entity)
{
    if (!entity.HasComponent<MeshRendererComponent>())
        return;

    bool removeComponent = false;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
    bool open = ImGui::CollapsingHeader("Mesh Renderer", flags);

    // Remove button on same line
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
    if (ImGui::Button(ICON_FA_TIMES "##RemoveMeshRenderer"))
    {
        removeComponent = true;
    }

    if (open && !removeComponent)
    {
        auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();

        ImGui::Indent();

        // Mesh reference display with drag & drop
        {
            const char* meshName = meshRenderer.mesh ? "Assigned" : "None (Drop Model Here)";

            ImGui::Text("Mesh:");
            ImGui::SameLine();

            // Make a drop target button
            ImVec4 buttonColor = meshRenderer.mesh
                ? ImVec4(0.2f, 0.4f, 0.2f, 1.0f)
                : ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);

            ImGui::Button(meshName, ImVec2(ImGui::GetContentRegionAvail().x, 0));

            ImGui::PopStyleColor();

            // Drag & Drop target for Model assets
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MODEL"))
                {
                    std::string assetPath(static_cast<const char*>(payload->Data));

                    // Load the model and assign mesh
                    hs::Model* model = AssetDatabase::Get().LoadModel(assetPath);
                    if (model)
                    {
                        meshRenderer.mesh = model->GetMesh();

                        // Also assign material if available and no materials set
                        if (meshRenderer.materials.empty() && model->GetMaterial())
                        {
                            meshRenderer.materials.push_back(model->GetMaterial());
                        }

                        // Update bounds from mesh if available
                        // TODO: Get bounds from mesh when Mesh::GetBounds() is implemented
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }

        // Materials section
        if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen))
        {
            uint32 materialCount = meshRenderer.GetMaterialCount();
            if (materialCount == 0)
            {
                ImGui::TextDisabled("No materials");
            }
            else
            {
                for (uint32 i = 0; i < materialCount; ++i)
                {
                    ImGui::PushID(static_cast<int>(i));

                    Material* mat = meshRenderer.GetMaterial(i);
                    const char* matName = mat ? "Assigned" : "None";

                    char label[32];
                    snprintf(label, sizeof(label), "[%u]", i);
                    ImGui::Text("%s %s", label, matName);

                    ImGui::PopID();
                }
            }

            // Add material slot button
            if (ImGui::Button("+ Add Material Slot"))
            {
                meshRenderer.materials.push_back(nullptr);
            }

            ImGui::TreePop();
        }

        ImGui::Separator();

        // Rendering flags
        ImGui::Checkbox("Visible", &meshRenderer.isVisible);
        ImGui::Checkbox("Cast Shadow", &meshRenderer.castShadow);
        ImGui::Checkbox("Receive Shadow", &meshRenderer.receiveShadow);

        // Render layer mask
        ImGui::Separator();
        int layerMask = static_cast<int>(meshRenderer.renderLayerMask);
        if (ImGui::InputInt("Render Layer Mask", &layerMask, 1, 100, ImGuiInputTextFlags_CharsHexadecimal))
        {
            meshRenderer.renderLayerMask = static_cast<uint32>(layerMask);
        }

        // Bounds info (read-only)
        if (ImGui::TreeNode("Bounds (Debug)"))
        {
            ImGui::Text("Local Min: (%.2f, %.2f, %.2f)",
                meshRenderer.localBounds.min.x,
                meshRenderer.localBounds.min.y,
                meshRenderer.localBounds.min.z);
            ImGui::Text("Local Max: (%.2f, %.2f, %.2f)",
                meshRenderer.localBounds.max.x,
                meshRenderer.localBounds.max.y,
                meshRenderer.localBounds.max.z);
            ImGui::Text("World Min: (%.2f, %.2f, %.2f)",
                meshRenderer.worldBounds.min.x,
                meshRenderer.worldBounds.min.y,
                meshRenderer.worldBounds.min.z);
            ImGui::Text("World Max: (%.2f, %.2f, %.2f)",
                meshRenderer.worldBounds.max.x,
                meshRenderer.worldBounds.max.y,
                meshRenderer.worldBounds.max.z);
            ImGui::TreePop();
        }

        ImGui::Unindent();
    }

    if (removeComponent)
    {
        entity.RemoveComponent<MeshRendererComponent>();
    }
}

void InspectorPanel::drawCameraComponent(Entity entity)
{
    if (!entity.HasComponent<CameraComponent>())
        return;

    bool removeComponent = false;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
    bool open = ImGui::CollapsingHeader("Camera", flags);

    // Remove button on same line
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
    if (ImGui::Button(ICON_FA_TIMES "##RemoveCamera"))
    {
        removeComponent = true;
    }

    if (open && !removeComponent)
    {
        auto& camera = entity.GetComponent<CameraComponent>();

        ImGui::Indent();

        // Projection type
        const char* projectionTypes[] = { "Perspective", "Orthographic" };
        int currentProjection = static_cast<int>(camera.projectionType);
        if (ImGui::Combo("Projection", &currentProjection, projectionTypes, 2))
        {
            camera.projectionType = static_cast<CameraComponent::EProjectionType>(currentProjection);
        }

        // Perspective settings
        if (camera.projectionType == CameraComponent::EProjectionType::Perspective)
        {
            ImGui::DragFloat("FOV", &camera.fov, 0.5f, 1.0f, 179.0f, "%.1f");
        }
        else
        {
            ImGui::DragFloat("Ortho Size", &camera.orthoSize, 0.1f, 0.1f, 1000.0f, "%.1f");
        }

        // Common settings
        ImGui::DragFloat("Near", &camera.nearPlane, 0.01f, 0.001f, camera.farPlane - 0.01f, "%.3f");
        ImGui::DragFloat("Far", &camera.farPlane, 1.0f, camera.nearPlane + 0.01f, 100000.0f, "%.1f");

        // Flags
        ImGui::Checkbox("Primary", &camera.isPrimary);
        ImGui::Checkbox("Active", &camera.isActive);

        ImGui::Unindent();
    }

    if (removeComponent)
    {
        entity.RemoveComponent<CameraComponent>();
    }
}

void InspectorPanel::drawLightComponent(Entity entity)
{
    if (!entity.HasComponent<LightComponent>())
        return;

    bool removeComponent = false;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
    bool open = ImGui::CollapsingHeader("Light", flags);

    // Remove button on same line
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
    if (ImGui::Button(ICON_FA_TIMES "##RemoveLight"))
    {
        removeComponent = true;
    }

    if (open && !removeComponent)
    {
        auto& light = entity.GetComponent<LightComponent>();

        ImGui::Indent();

        // Light type
        const char* lightTypes[] = { "Directional", "Point", "Spot" };
        int currentType = static_cast<int>(light.type);
        if (ImGui::Combo("Type", &currentType, lightTypes, 3))
        {
            light.type = static_cast<ELightType>(currentType);
        }

        // Color
        drawColorEdit("Color", light.color);

        // Intensity
        ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 100.0f, "%.2f");

        // Point/Spot light settings
        if (light.type != ELightType::Directional)
        {
            ImGui::DragFloat("Range", &light.range, 0.1f, 0.1f, 1000.0f, "%.1f");
            ImGui::DragFloat("Attenuation", &light.attenuation, 0.01f, 0.0f, 10.0f, "%.2f");
        }

        // Spot light settings
        if (light.type == ELightType::Spot)
        {
            ImGui::DragFloat("Inner Cone", &light.innerConeAngle, 0.5f, 1.0f, light.outerConeAngle - 1.0f, "%.1f");
            ImGui::DragFloat("Outer Cone", &light.outerConeAngle, 0.5f, light.innerConeAngle + 1.0f, 90.0f, "%.1f");
        }

        ImGui::Separator();

        // Shadow settings
        ImGui::Checkbox("Cast Shadow", &light.castShadow);
        if (light.castShadow)
        {
            ImGui::DragFloat("Shadow Bias", &light.shadowBias, 0.0001f, 0.0f, 0.1f, "%.4f");

            // Shadow map resolution
            const char* resolutions[] = { "512", "1024", "2048", "4096" };
            int currentRes = 1; // Default to 1024
            if (light.shadowMapResolution == 512) currentRes = 0;
            else if (light.shadowMapResolution == 1024) currentRes = 1;
            else if (light.shadowMapResolution == 2048) currentRes = 2;
            else if (light.shadowMapResolution == 4096) currentRes = 3;

            if (ImGui::Combo("Shadow Resolution", &currentRes, resolutions, 4))
            {
                const uint32 resValues[] = { 512, 1024, 2048, 4096 };
                light.shadowMapResolution = resValues[currentRes];
            }
        }

        // Enabled flag
        ImGui::Checkbox("Enabled", &light.isEnabled);

        ImGui::Unindent();
    }

    if (removeComponent)
    {
        entity.RemoveComponent<LightComponent>();
    }
}

bool InspectorPanel::drawVec3Control(const char* label, glm::vec3& values, float resetValue, float speed)
{
    bool modified = false;

    ImGui::PushID(label);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100.0f);
    ImGui::Text("%s", label);
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    // X (Red)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    if (ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##X", &values.x, speed, 0.0f, 0.0f, "%.3f"))
    {
        modified = true;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Y (Green)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    if (ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##Y", &values.y, speed, 0.0f, 0.0f, "%.3f"))
    {
        modified = true;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Z (Blue)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    if (ImGui::Button("Z", buttonSize))
    {
        values.z = resetValue;
        modified = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::DragFloat("##Z", &values.z, speed, 0.0f, 0.0f, "%.3f"))
    {
        modified = true;
    }
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);

    ImGui::PopID();

    return modified;
}

void InspectorPanel::drawColorEdit(const char* label, glm::vec3& color)
{
    ImGui::ColorEdit3(label, &color.x, ImGuiColorEditFlags_Float);
}

void InspectorPanel::drawAddComponentButton(Entity entity)
{
    float width = ImGui::GetContentRegionAvail().x;
    float buttonWidth = 200.0f;
    ImGui::SetCursorPosX((width - buttonWidth) / 2.0f);

    if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0)))
    {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        if (!entity.HasComponent<MeshRendererComponent>())
        {
            if (ImGui::MenuItem("Mesh Renderer"))
            {
                entity.AddComponent<MeshRendererComponent>();
                ImGui::CloseCurrentPopup();
            }
        }

        if (!entity.HasComponent<CameraComponent>())
        {
            if (ImGui::MenuItem("Camera"))
            {
                entity.AddComponent<CameraComponent>();
                ImGui::CloseCurrentPopup();
            }
        }

        if (!entity.HasComponent<LightComponent>())
        {
            if (ImGui::MenuItem("Light"))
            {
                entity.AddComponent<LightComponent>();
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}

HS_NS_EDITOR_END
