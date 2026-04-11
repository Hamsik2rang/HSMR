//
//  InspectorPanel.cpp
//  Editor
//
//  Component editor panel for selected entity
//

#include "Editor/Panel/InspectorPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Asset/AssetDatabase.h"
#include "Editor/GUI/EditorFeedbackWidgets.h"
#include "Editor/GUI/EditorFormLayout.h"
#include "Editor/GUI/EditorIcons.h"
#include "Editor/Panel/EditorPanelFrame.h"
#include "Editor/Project/ProjectContext.h"

#include "Scene/Scene.h"
#include "Scene/Components/Components.h"

#include "Core/HAL/FileDialog.h"
#include "Core/HAL/FileSystem.h"
#include "Resource/ObjectManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Resource/MaterialTextureBinding.h"
#include "Resource/Shader.h"

#include "ImGui/imgui_internal.h"

#include <cstring>

HS_NS_EDITOR_BEGIN

namespace
{
bool drawRemovableComponentHeader(const char* label, const char* removeId, bool& outRemove)
{
    const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
    const bool open = ImGui::CollapsingHeader(label, flags);

    const ImVec2 headerMin = ImGui::GetItemRectMin();
    const ImVec2 headerMax = ImGui::GetItemRectMax();
    const ImVec2 restoreCursor = ImGui::GetCursorScreenPos();

    const float headerHeight = headerMax.y - headerMin.y;
    const float buttonEdge = ImMax(14.0f, headerHeight - 6.0f);
    const ImVec2 buttonSize(buttonEdge, buttonEdge);
    const float buttonPaddingX = ImGui::GetStyle().FramePadding.x;
    const float buttonPosX = headerMax.x - buttonSize.x - buttonPaddingX;
    const float buttonPosY = headerMin.y + (headerHeight - buttonSize.y) * 0.5f;

    ImGui::SetCursorScreenPos(ImVec2(buttonPosX, buttonPosY));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 32));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 64));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

    std::string labelWithId = std::string(EditorIcons::Close) + "##" + removeId;
    if (ImGui::Button(labelWithId.c_str(), buttonSize))
    {
        outRemove = true;
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::SetCursorScreenPos(restoreCursor);
    return open;
}

const char* getMaterialTextureLabel(EMaterialTextureType type)
{
    switch (type)
    {
    case EMaterialTextureType::Diffuse:         return "Diffuse";
    case EMaterialTextureType::Specular:        return "Specular";
    case EMaterialTextureType::Normal:          return "Normal";
    case EMaterialTextureType::Emission:        return "Emission";
    case EMaterialTextureType::Ambient:         return "Ambient";
    case EMaterialTextureType::Roughness:       return "Roughness";
    case EMaterialTextureType::Metallic:        return "Metallic";
    case EMaterialTextureType::AmbientOcclusion:return "Ambient Occlusion";
    default:                                    return "Texture";
    }
}

const char* getMaterialDisplayName(Material* material)
{
    if (!material)
    {
        return "None";
    }

    if (material->name && material->name[0] != '\0')
    {
        return material->name;
    }

    return nullptr;
}

std::string sanitizeMaterialAssetName(const std::string& name)
{
    std::string result;
    result.reserve(name.size());
    for (char c : name)
    {
        if (std::isalnum(static_cast<unsigned char>(c)))
        {
            result.push_back(c);
        }
        else if (c == ' ' || c == '_' || c == '-')
        {
            result.push_back('_');
        }
    }

    if (result.empty())
    {
        result = "Material";
    }
    return result;
}

std::string buildUniqueMaterialRelativePath(const std::string& baseName)
{
    const std::string& currentFolder = EditorContext::Get().GetCurrentAssetFolderPath();
    const std::string prefix = currentFolder.empty() ? "" : currentFolder + "/";
    std::string relativePath = prefix + baseName + ".mat";
    int suffix = 1;
    while (AssetDatabase::Get().FindAsset(relativePath))
    {
        relativePath = prefix + baseName + "_" + std::to_string(suffix++) + ".mat";
    }
    return relativePath;
}

std::string makeAssetRelativePath(const std::string& absolutePath)
{
    if (!ProjectContext::Get().IsProjectOpen())
    {
        return "";
    }

    const std::filesystem::path assetRoot = std::filesystem::weakly_canonical(ProjectContext::Get().GetAssetPath());
    const std::filesystem::path targetPath = std::filesystem::weakly_canonical(absolutePath);
    std::error_code ec;
    std::filesystem::path relativePath = std::filesystem::relative(targetPath, assetRoot, ec);
    if (ec || relativePath.empty())
    {
        return "";
    }

    return relativePath.generic_string();
}

std::string toLowerString(const std::string& value)
{
    std::string lower = value;
    for (char& c : lower)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}

bool isColorParameterName(const std::string& name)
{
    const std::string lowerName = toLowerString(name);
    return lowerName.find("color") != std::string::npos ||
           lowerName.find("tint") != std::string::npos ||
           lowerName.find("albedo") != std::string::npos;
}

float getFloatParameterSpeed(const std::string& name)
{
    const std::string lowerName = toLowerString(name);
    if (lowerName.find("opacity") != std::string::npos ||
        lowerName.find("roughness") != std::string::npos ||
        lowerName.find("metallic") != std::string::npos)
    {
        return 0.01f;
    }

    if (lowerName.find("shininess") != std::string::npos)
    {
        return 0.1f;
    }

    return 0.01f;
}

void syncCommonMaterialState(Material* material, const std::string& memberName, const glm::vec4& value)
{
    if (!material)
    {
        return;
    }

    if (memberName == "diffuseColor")
    {
        material->SetDiffuseColor(value);
    }
    else if (memberName == "specularColor")
    {
        material->SetSpecularColor(value);
    }
    else if (memberName == "emissionColor")
    {
        material->SetEmissionColor(value);
    }
    else if (memberName == "ambientColor")
    {
        material->SetAmbientColor(value);
    }
}

void syncCommonMaterialParameter(Material* material, const char* name, const glm::vec4& value)
{
    if (material)
    {
        material->SetParameter(name, value);
    }
}

void syncCommonMaterialParameter(Material* material, const char* name, float value)
{
    if (material)
    {
        material->SetParameter(name, value);
    }
}

void syncCommonMaterialParameter(Material* material, const char* name, int32 value)
{
    if (material)
    {
        material->SetParameter(name, value);
    }
}

void syncCommonMaterialState(Material* material, const std::string& memberName, float value)
{
    if (!material)
    {
        return;
    }

    if (memberName == "shininess")
    {
        material->SetShininess(value);
    }
    else if (memberName == "opacity")
    {
        material->SetOpacity(value);
    }
    else if (memberName == "roughness")
    {
        material->SetRoughness(value);
    }
    else if (memberName == "metallic")
    {
        material->SetMetallic(value);
    }
}

void syncCommonMaterialState(Material* material, const std::string& memberName, int32 value)
{
    if (!material)
    {
        return;
    }

    if (memberName == "twoSided")
    {
        material->SetTwoSided(value != 0);
    }
}
}
InspectorPanel::InspectorPanel(Window* window, const char* panelId)
    : Panel(window, panelId)
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
    if (!IsVisible())
    {
        return;
    }

    EditorPanelWindowOptions panelOptions{};
    panelOptions.pOpen = GetVisibilityBinding();
    EditorPanelFrame::BeginStandardPanel("Inspector", panelOptions);

    Entity selectedEntity = EditorContext::Get().GetSelectedEntity();
    const std::string& selectedAssetPath = EditorContext::Get().GetSelectedAssetPath();

    if (!selectedEntity.IsValid())
    {
        if (!selectedAssetPath.empty())
        {
            const AssetEntry* asset = AssetDatabase::Get().FindAsset(selectedAssetPath);
            if (asset && asset->type == EAssetType::Material)
            {
                drawMaterialAssetInspector(selectedAssetPath);
                EditorPanelFrame::EndStandardPanel();
                return;
            }
        }

        EditorFeedbackWidgets::EmptyState("No entity selected");
        EditorPanelFrame::EndStandardPanel();
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

    EditorPanelFrame::EndStandardPanel();
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
        if (EditorFormLayout::Begin("TagProperties"))
        {
            int layer = static_cast<int>(tag.layer);
            if (EditorFormLayout::InputIntRow("Layer", &layer))
            {
                tag.layer = static_cast<uint32>(glm::max(0, layer));
            }

            EditorFormLayout::CheckboxRow("Static", &tag.isStatic);
            EditorFormLayout::CheckboxRow("Active", &tag.isActive);

            EditorFormLayout::End();
        }
    }
}

void InspectorPanel::drawTransformComponent(Entity entity)
{
    if (!entity.HasComponent<TransformComponent>())
        return;

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& transform = entity.GetComponent<TransformComponent>();

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

        ImGui::Separator();
        if (EditorFormLayout::Begin("TransformReadOnly"))
        {
            glm::vec3 worldPosition = transform.GetWorldPosition();
            EditorFormLayout::BeginRow("World Position");
            ImGui::Text("%.3f, %.3f, %.3f", worldPosition.x, worldPosition.y, worldPosition.z);

            EditorFormLayout::BeginRow("Children");
            ImGui::Text("%zu", transform.children.size());

            EditorFormLayout::BeginRow("Parent");
            if (transform.HasParent())
            {
                ImGui::Text("%u", static_cast<uint32>(transform.parent));
            }
            else
            {
                ImGui::TextDisabled("None");
            }

            EditorFormLayout::End();
        }
    }
}

void InspectorPanel::drawMeshRendererComponent(Entity entity)
{
    if (!entity.HasComponent<MeshRendererComponent>())
        return;

    bool removeComponent = false;
    bool open = EditorWidgets::BeginRemovableSectionHeader("Mesh Renderer", "RemoveMeshRenderer", removeComponent);

    if (open && !removeComponent)
    {
        auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();

        // Mesh reference display with drag & drop
        {
            const char* meshName = meshRenderer.mesh ? "Assigned" : "None (Drop Model Here)";
            if (EditorFormLayout::Begin("MeshRendererHeader"))
            {
                EditorFormLayout::BeginRow("Mesh");

                ImVec4 buttonColor = meshRenderer.mesh
                                         ? ImVec4(0.2f, 0.4f, 0.2f, 1.0f)
                                         : ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                ImGui::Button(meshName, ImVec2(-FLT_MIN, 0.0f));
                ImGui::PopStyleColor();

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MODEL"))
                    {
                        std::string assetPath(static_cast<const char*>(payload->Data));

                        hs::Model* model = AssetDatabase::Get().LoadModel(assetPath);
                        if (model)
                        {
                            meshRenderer.mesh = model->GetMesh();
                            if (meshRenderer.mesh)
                            {
                                const auto& bound        = meshRenderer.mesh->GetBound();
                                meshRenderer.localBounds = AABB(glm::vec3(bound.min), glm::vec3(bound.max));
                                meshRenderer.boundsDirty = true;
                            }

                            if (meshRenderer.materials.empty() && model->GetMaterial())
                            {
                                meshRenderer.materials.push_back(model->GetMaterial());
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                EditorFormLayout::End();
            }
        }

        // Materials section
        if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (meshRenderer.materials.empty())
            {
                ImGui::TextDisabled("No materials");
            }
            else
            {
                int removeSlotIndex = -1;
                for (uint32 i = 0; i < meshRenderer.materials.size(); ++i)
                {
                    ImGui::PushID(static_cast<int>(i));
                    Material* material = meshRenderer.materials[i];
                    const char* materialName = getMaterialDisplayName(material);

                    char headerLabel[128];
                    if (material)
                    {
                        snprintf(
                            headerLabel,
                            sizeof(headerLabel),
                            "[%u] %s##MaterialSlot_%u",
                            i,
                            materialName ? materialName : ("Material"),
                            i);
                    }
                    else
                    {
                        snprintf(headerLabel, sizeof(headerLabel), "[%u] None##MaterialSlot_%u", i, i);
                    }

                    bool slotOpen = ImGui::TreeNodeEx(headerLabel, ImGuiTreeNodeFlags_DefaultOpen);
                    ImGui::SameLine();

                    if (ImGui::SmallButton("Clear"))
                    {
                        meshRenderer.materials[i] = nullptr;
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                    {
                        removeSlotIndex = static_cast<int>(i);
                    }

                    if (slotOpen)
                    {
                        drawMaterialSlotEditor(meshRenderer, i);
                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }

                if (removeSlotIndex >= 0)
                {
                    meshRenderer.materials.erase(meshRenderer.materials.begin() + removeSlotIndex);
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
        if (EditorFormLayout::Begin("MeshRendererFlags"))
        {
            EditorFormLayout::CheckboxRow("Visible", &meshRenderer.isVisible);
            EditorFormLayout::CheckboxRow("Cast Shadow", &meshRenderer.castShadow);
            EditorFormLayout::CheckboxRow("Receive Shadow", &meshRenderer.receiveShadow);

            int layerMask = static_cast<int>(meshRenderer.renderLayerMask);
            if (EditorFormLayout::InputIntRow("Render Layer Mask", &layerMask, ImGuiInputTextFlags_CharsHexadecimal))
            {
                meshRenderer.renderLayerMask = static_cast<uint32>(layerMask);
            }

            EditorFormLayout::End();
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
    bool open = EditorWidgets::BeginRemovableSectionHeader("Camera", "RemoveCamera", removeComponent);

    if (open && !removeComponent)
    {
        auto& camera = entity.GetComponent<CameraComponent>();

        const char* projectionTypes[] = {"Perspective", "Orthographic"};
        int currentProjection         = static_cast<int>(camera.projectionType);
        if (EditorFormLayout::Begin("CameraProperties"))
        {
            if (EditorFormLayout::ComboRow("Projection", &currentProjection, projectionTypes, 2))
            {
                camera.projectionType = static_cast<CameraComponent::EProjectionType>(currentProjection);
            }

            if (camera.projectionType == CameraComponent::EProjectionType::Perspective)
            {
                EditorFormLayout::DragFloatRow("FOV", &camera.fov, 0.5f, 1.0f, 179.0f, "%.1f");
            }
            else
            {
                EditorFormLayout::DragFloatRow("Ortho Size", &camera.orthoSize, 0.1f, 0.1f, 1000.0f, "%.1f");
            }

            EditorFormLayout::DragFloatRow("Near", &camera.nearPlane, 0.01f, 0.001f, camera.farPlane - 0.01f, "%.3f");
            EditorFormLayout::DragFloatRow("Far", &camera.farPlane, 1.0f, camera.nearPlane + 0.01f, 100000.0f, "%.1f");
            EditorFormLayout::InputIntRow("Priority", &camera.priority);
            EditorFormLayout::CheckboxRow("Primary", &camera.isPrimary);
            EditorFormLayout::CheckboxRow("Active", &camera.isActive);
            EditorFormLayout::DragFloat4Row("Viewport", &camera.viewport.x, 0.01f, 0.0f, 1.0f, "%.2f");

            EditorFormLayout::End();
        }
    }

    if (removeComponent)
    {
        entity.RemoveComponent<CameraComponent>();
    }
}

void InspectorPanel::drawMaterialSlotEditor(MeshRendererComponent& meshRenderer, uint32 slotIndex)
{
    Material* material = slotIndex < meshRenderer.materials.size() ? meshRenderer.materials[slotIndex] : nullptr;

    if (!material)
    {
        if (ImGui::Button("Create Material"))
        {
            std::string suggestedName = "Material";
            if (EditorContext::Get().GetSelectedEntity().IsValid() &&
                EditorContext::Get().GetSelectedEntity().HasComponent<TagComponent>())
            {
                suggestedName = EditorContext::Get().GetSelectedEntity().GetComponent<TagComponent>().name;
            }

            meshRenderer.SetMaterial(createMaterialAsset(suggestedName, slotIndex), slotIndex);
            material = meshRenderer.GetMaterial(slotIndex);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Create and assign a material asset.");
    }

    const char* slotTargetLabel = material ? "Drop Material Asset Here" : "Drop Material Here";
    ImGui::Button((std::string(slotTargetLabel) + "##MaterialDropTarget_" + std::to_string(slotIndex)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0.0f));

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MATERIAL"))
        {
            std::string assetPath(static_cast<const char*>(payload->Data));
            if (Material* assetMaterial = AssetDatabase::Get().LoadMaterial(assetPath))
            {
                meshRenderer.SetMaterial(assetMaterial, slotIndex);
                material = meshRenderer.GetMaterial(slotIndex);
            }
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MODEL"))
        {
            std::string assetPath(static_cast<const char*>(payload->Data));
            hs::Model* model = AssetDatabase::Get().LoadModel(assetPath);
            if (model && model->GetMaterial())
            {
                meshRenderer.SetMaterial(model->GetMaterial(), slotIndex);
                material = meshRenderer.GetMaterial(slotIndex);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (!material)
    {
        return;
    }

    if (material->HasSourceAssetPath())
    {
        ImGui::TextDisabled("Asset: %s", material->GetSourceAssetPath().c_str());
        if (ImGui::SmallButton("Save"))
        {
            persistMaterialIfAssetBacked(material);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Save As..."))
        {
            saveMaterialAs(material, material->GetDisplayName().empty() ? "Material" : material->GetDisplayName());
        }
    }
    else
    {
        ImGui::TextDisabled("Unsaved Material");
        if (ImGui::SmallButton("Create Material Asset"))
        {
            std::string suggestedName = "Material";
            if (EditorContext::Get().GetSelectedEntity().IsValid() &&
                EditorContext::Get().GetSelectedEntity().HasComponent<TagComponent>())
            {
                suggestedName = EditorContext::Get().GetSelectedEntity().GetComponent<TagComponent>().name;
            }

            std::string baseName = sanitizeMaterialAssetName(suggestedName) + "_" + std::to_string(slotIndex);
            std::string relativePath = buildUniqueMaterialRelativePath(baseName);
            if (AssetDatabase::Get().SaveMaterial(relativePath, material))
            {
                if (Material* savedMaterial = AssetDatabase::Get().LoadMaterial(relativePath))
                {
                    material = savedMaterial;
                    meshRenderer.SetMaterial(material, slotIndex);
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Save As..."))
        {
            saveMaterialAs(material, "Material");
        }
    }

    char idSuffix[32];
    snprintf(idSuffix, sizeof(idSuffix), "slot_%u", slotIndex);
    drawMaterialEditor(material, idSuffix);
}

bool InspectorPanel::drawMaterialEditor(Material* material, const char* idSuffix)
{
    if (!material)
    {
        return false;
    }

    bool changed = false;

    Shader* shader = material->GetShader();
    if (shader && shader->IsCompiledEx() && !material->GetParameterBlock())
    {
        material->InitializeParameterBlock();
    }

    const char* shaderName = "Auto (renderer fallback)";
    if (shader)
    {
        shaderName = shader->GetShaderName().empty() ? "Unnamed Shader" : shader->GetShaderName().c_str();
    }

    ImGui::Text("Shader:");
    ImGui::SameLine();
    ImGui::TextDisabled("%s", shaderName);

    if (shader && shader->IsCompiledEx())
    {
        std::vector<EMaterialTextureType> reflectedTextureTypes;
        std::unordered_set<int> seenTextureTypes;
        for (const auto& textureBinding : shader->GetReflection().textureBindings)
        {
            EMaterialTextureType textureType = MapTextureBindingNameToMaterialTextureType(textureBinding.name);
            if (seenTextureTypes.insert(static_cast<int>(textureType)).second)
            {
                reflectedTextureTypes.push_back(textureType);
            }
        }

        if (!reflectedTextureTypes.empty())
        {
            ImGui::SeparatorText("Textures");
            if (EditorFormLayout::Begin(("MaterialTextures_" + std::string(idSuffix)).c_str()))
            {
                for (EMaterialTextureType textureType : reflectedTextureTypes)
                {
                    changed |= drawMaterialTextureSlot(material, textureType, getMaterialTextureLabel(textureType), idSuffix);
                }
                EditorFormLayout::End();
            }
        }

        ImGui::SeparatorText("Parameters");
        changed |= drawMaterialParameterBlockEditor(material, idSuffix);
    }
    else
    {
        ImGui::SeparatorText("Textures");
        if (EditorFormLayout::Begin(("MaterialTexturesFallback_" + std::string(idSuffix)).c_str()))
        {
            changed |= drawMaterialTextureSlot(material, EMaterialTextureType::Diffuse, getMaterialTextureLabel(EMaterialTextureType::Diffuse), idSuffix);
            EditorFormLayout::End();
        }
        ImGui::SeparatorText("Parameters");
        ImGui::TextDisabled("No reflected material parameters");
    }

    if (changed)
    {
        persistMaterialIfAssetBacked(material);
    }

    return changed;
}

bool InspectorPanel::drawMaterialTextureSlot(Material* material, EMaterialTextureType type, const char* label, const char* idSuffix)
{
    Image* image = material ? material->GetTexture(type) : nullptr;
    bool changed = false;

    EditorFormLayout::BeginRow(label);

    std::string buttonLabel;
    if (image && image->name)
    {
        buttonLabel = std::string(image->name) + "##" + label + "_" + idSuffix;
    }
    else if (image)
    {
        buttonLabel = "Assigned##" + std::string(label) + "_" + idSuffix;
    }
    else
    {
        buttonLabel = "None (Drop Texture Here)##" + std::string(label) + "_" + idSuffix;
    }

    const float clearButtonWidth = 32.0f;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    ImGui::Button(buttonLabel.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - clearButtonWidth - spacing, 0.0f));

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_TEXTURE"))
        {
            std::string assetPath(static_cast<const char*>(payload->Data));
            if (Image* texture = AssetDatabase::Get().LoadTexture(assetPath))
            {
                material->SetTexture(type, texture);
                material->SetTextureAssetPath(type, assetPath);
                HS_LOG(
                    info,
                    "[InspectorPanel] Assigned texture '{}' to material '{}' slot '{}' (ptr={})",
                    assetPath.c_str(),
                    material->GetDisplayName().c_str(),
                    label,
                    static_cast<const void*>(texture));
                changed = true;
            }
            else
            {
                HS_LOG(warning, "[InspectorPanel] Failed to load texture asset '{}'", assetPath.c_str());
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::SameLine();
    if (ImGui::SmallButton((std::string("X##ClearTex_") + label + "_" + idSuffix).c_str()))
    {
        material->SetTexture(type, nullptr);
        material->SetTextureAssetPath(type, "");
        changed = true;
    }

    return changed;
}

bool InspectorPanel::drawMaterialParameterBlockEditor(Material* material, const char* idSuffix)
{
    if (!material)
    {
        return false;
    }

    bool changed = false;

    MaterialParameterBlock* parameterBlock = material->GetParameterBlock();
    Shader* shader = material->GetShader();
    if (!parameterBlock || !shader || !shader->IsCompiledEx())
    {
        ImGui::TextDisabled("No per-material parameter block");
        return false;
    }

    const ShaderBufferBindingInfo* bufferInfo = shader->GetReflection().FindBuffer(parameterBlock->GetBufferName());
    if (!bufferInfo)
    {
        ImGui::TextDisabled("No reflected per-material buffer");
        return false;
    }

    const uint8* parameterData = static_cast<const uint8*>(parameterBlock->GetData());
    if (!parameterData)
    {
        ImGui::TextDisabled("Parameter block is empty");
        return false;
    }

    bool anyEditableMembers = false;
    if (EditorFormLayout::Begin(("MaterialParameters_" + std::string(idSuffix)).c_str()))
    {
        for (const auto& member : bufferInfo->members)
        {
            const uint8* memberData = parameterData + member.offset;
            std::string widgetId = "##" + member.name + "_" + idSuffix;

            switch (member.category)
            {
            case ShaderBufferMember::Category::Scalar:
                if (member.baseType == ShaderBufferMember::BaseType::Float && member.size >= sizeof(float))
                {
                    float value = *reinterpret_cast<const float*>(memberData);
                    EditorFormLayout::BeginRow(member.name.c_str());
                    if (ImGui::DragFloat(widgetId.c_str(), &value, getFloatParameterSpeed(member.name)))
                    {
                        material->SetParameter(member.name, value);
                        syncCommonMaterialState(material, member.name, value);
                        changed = true;
                    }
                    anyEditableMembers = true;
                }
                else if (member.baseType == ShaderBufferMember::BaseType::Int && member.size >= sizeof(int32))
                {
                    int32 value = *reinterpret_cast<const int32*>(memberData);
                    EditorFormLayout::BeginRow(member.name.c_str());
                    if (member.name == "twoSided")
                    {
                        bool enabled = value != 0;
                        if (ImGui::Checkbox(widgetId.c_str(), &enabled))
                        {
                            value = enabled ? 1 : 0;
                            material->SetParameter(member.name, value);
                            syncCommonMaterialState(material, member.name, value);
                            changed = true;
                        }
                    }
                    else if (ImGui::DragInt(widgetId.c_str(), &value, 1.0f))
                    {
                        material->SetParameter(member.name, value);
                        syncCommonMaterialState(material, member.name, value);
                        changed = true;
                    }
                    anyEditableMembers = true;
                }
                break;
            case ShaderBufferMember::Category::Vector:
                if (member.baseType == ShaderBufferMember::BaseType::Float)
                {
                    EditorFormLayout::BeginRow(member.name.c_str());
                    if (member.rowCount == 2 && member.size >= sizeof(glm::vec2))
                    {
                        glm::vec2 value = *reinterpret_cast<const glm::vec2*>(memberData);
                        if (ImGui::DragFloat2(widgetId.c_str(), &value.x, 0.01f))
                        {
                            material->SetParameter(member.name, value);
                            changed = true;
                        }
                        anyEditableMembers = true;
                    }
                    else if (member.rowCount == 3 && member.size >= sizeof(glm::vec3))
                    {
                        glm::vec3 value = *reinterpret_cast<const glm::vec3*>(memberData);
                        if (isColorParameterName(member.name)
                            ? ImGui::ColorEdit3(widgetId.c_str(), &value.x, ImGuiColorEditFlags_DisplayRGB)
                            : ImGui::DragFloat3(widgetId.c_str(), &value.x, 0.01f))
                        {
                            material->SetParameter(member.name, value);
                            changed = true;
                        }
                        anyEditableMembers = true;
                    }
                    else if (member.rowCount == 4 && member.size >= sizeof(glm::vec4))
                    {
                        glm::vec4 value = *reinterpret_cast<const glm::vec4*>(memberData);
                        if (isColorParameterName(member.name)
                            ? ImGui::ColorEdit4(widgetId.c_str(), &value.x, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar)
                            : ImGui::DragFloat4(widgetId.c_str(), &value.x, 0.01f))
                        {
                            material->SetParameter(member.name, value);
                            syncCommonMaterialState(material, member.name, value);
                            changed = true;
                        }
                        anyEditableMembers = true;
                    }
                }
                break;
            default:
                break;
            }
        }
        EditorFormLayout::End();
    }

    if (!anyEditableMembers)
    {
        ImGui::TextDisabled("No editable reflected parameters");
    }

    return changed;
}

Material* InspectorPanel::createRuntimeMaterial()
{
    Scoped<Material> material = MakeScoped<Material>();
    material->SetDisplayName("Runtime Material");
    material->SetTexture(EMaterialTextureType::Diffuse, const_cast<Image*>(ObjectManager::GetFallbackImage2DWhite()));
    material->SetDiffuseColor(glm::vec4(1.0f));

    Material* materialPtr = material.get();
    _runtimeMaterials.push_back(std::move(material));
    return materialPtr;
}

Material* InspectorPanel::createMaterialAsset(const std::string& suggestedName, uint32 slotIndex)
{
    std::string baseName = sanitizeMaterialAssetName(suggestedName) + "_" + std::to_string(slotIndex);
    std::string relativePath = buildUniqueMaterialRelativePath(baseName);

    Scoped<Material> material = MakeScoped<Material>();
    material->SetDisplayName(std::filesystem::path(relativePath).stem().string());
    material->SetTexture(EMaterialTextureType::Diffuse, const_cast<Image*>(ObjectManager::GetFallbackImage2DWhite()));
    material->SetDiffuseColor(glm::vec4(1.0f));

    if (!AssetDatabase::Get().SaveMaterial(relativePath, material.get()))
    {
        return nullptr;
    }

    return AssetDatabase::Get().LoadMaterial(relativePath);
}

bool InspectorPanel::saveMaterialAs(Material* material, const std::string& suggestedName)
{
    if (!material || !ProjectContext::Get().IsProjectOpen())
    {
        return false;
    }

    hs::FileDialogFilter filters[] = {
        {"Material Files", "*.mat"},
        {"All Files", "*.*"}
    };

    std::string defaultLocation = ProjectContext::Get().GetAssetPath() + "Materials";
    std::string savePath = hs::FileDialog::SaveFile(filters, 2, defaultLocation.c_str());
    if (savePath.empty())
    {
        return false;
    }

    if (std::filesystem::path(savePath).extension() != ".mat")
    {
        savePath += ".mat";
    }

    std::string relativePath = makeAssetRelativePath(savePath);
    if (relativePath.empty())
    {
        return false;
    }

    material->SetDisplayName(std::filesystem::path(relativePath).stem().string());
    if (!AssetDatabase::Get().SaveMaterial(relativePath, material))
    {
        return false;
    }

    material->SetSourceAssetPath(relativePath);
    EditorContext::Get().SetSelectedAssetPath(relativePath);
    return true;
}

void InspectorPanel::persistMaterialIfAssetBacked(Material* material)
{
    if (!material || !material->HasSourceAssetPath())
    {
        return;
    }

    AssetDatabase::Get().SaveMaterial(material->GetSourceAssetPath(), material);
}

void InspectorPanel::drawMaterialAssetInspector(const std::string& assetPath)
{
    Material* material = AssetDatabase::Get().LoadMaterial(assetPath);
    if (!material)
    {
        ImGui::TextDisabled("Failed to load material asset");
        return;
    }

    ImGui::Text("Material Asset");
    ImGui::TextDisabled("%s", assetPath.c_str());
    if (ImGui::Button("Save"))
    {
        persistMaterialIfAssetBacked(material);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save As..."))
    {
        saveMaterialAs(material, material->GetDisplayName().empty() ? "Material" : material->GetDisplayName());
    }

    ImGui::Separator();
    drawMaterialEditor(material, "asset_material");
}

void InspectorPanel::drawLightComponent(Entity entity)
{
    if (!entity.HasComponent<LightComponent>())
        return;

    bool removeComponent = false;
    bool open = EditorWidgets::BeginRemovableSectionHeader("Light", "RemoveLight", removeComponent);

    if (open && !removeComponent)
    {
        auto& light = entity.GetComponent<LightComponent>();

        const char* lightTypes[] = {"Directional", "Point", "Spot"};
        int currentType          = static_cast<int>(light.type);
        if (EditorFormLayout::Begin("LightProperties"))
        {
            if (EditorFormLayout::ComboRow("Type", &currentType, lightTypes, 3))
            {
                light.type = static_cast<ELightType>(currentType);
            }

            drawColorEdit("Color", light.color);
            EditorFormLayout::DragFloatRow("Intensity", &light.intensity, 0.01f, 0.0f, 100.0f, "%.2f");

            if (light.type != ELightType::Directional)
            {
                EditorFormLayout::DragFloatRow("Range", &light.range, 0.1f, 0.1f, 1000.0f, "%.1f");
                EditorFormLayout::DragFloatRow("Attenuation", &light.attenuation, 0.01f, 0.0f, 10.0f, "%.2f");
            }

            if (light.type == ELightType::Spot)
            {
                EditorFormLayout::DragFloatRow("Inner Cone", &light.innerConeAngle, 0.5f, 1.0f, light.outerConeAngle - 1.0f, "%.1f");
                EditorFormLayout::DragFloatRow("Outer Cone", &light.outerConeAngle, 0.5f, light.innerConeAngle + 1.0f, 90.0f, "%.1f");
            }

            EditorFormLayout::CheckboxRow("Cast Shadow", &light.castShadow);
            if (light.castShadow)
            {
                EditorFormLayout::DragFloatRow("Shadow Bias", &light.shadowBias, 0.0001f, 0.0f, 0.1f, "%.4f");

                const char* resolutions[] = {"512", "1024", "2048", "4096"};
                int currentRes            = 1;
                if (light.shadowMapResolution == 512) currentRes = 0;
                else if (light.shadowMapResolution == 1024) currentRes = 1;
                else if (light.shadowMapResolution == 2048) currentRes = 2;
                else if (light.shadowMapResolution == 4096) currentRes = 3;

                if (EditorFormLayout::ComboRow("Shadow Resolution", &currentRes, resolutions, 4))
                {
                    const uint32 resValues[]  = {512, 1024, 2048, 4096};
                    light.shadowMapResolution = resValues[currentRes];
                }
            }

            EditorFormLayout::CheckboxRow("Enabled", &light.isEnabled);
            EditorFormLayout::End();
        }
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
    if (!EditorFormLayout::Begin("##Vec3Control", 130.0f))
    {
        ImGui::PopID();
        return false;
    }
    EditorFormLayout::BeginRow(label);

    float lineHeight  = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = {lineHeight, lineHeight};

    ImGui::PushMultiItemsWidths(3, ImGui::GetContentRegionAvail().x - 3.0f * buttonSize.x + 2.0f * GImGui->Style.ItemInnerSpacing.x);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

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
    EditorFormLayout::End();

    ImGui::PopID();

    return modified;
}

void InspectorPanel::drawColorEdit(const char* label, glm::vec3& color)
{
    EditorFormLayout::BeginRow(label);
    std::string id = std::string("##") + label;
    ImGui::ColorEdit3(id.c_str(), &color.x, ImGuiColorEditFlags_Float);
}

void InspectorPanel::drawAddComponentButton(Entity entity)
{
    float width       = ImGui::GetContentRegionAvail().x;
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
