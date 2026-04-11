//
//  HierarchyPanel.cpp
//  Editor
//
//  Entity hierarchy tree view with selection support
//

#include "Editor/Panel/HierarchyPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Asset/AssetDatabase.h"
#include "Editor/GUI/EditorIcons.h"
#include "Editor/GUI/EditorTreeWidgets.h"
#include "Editor/Panel/EditorPanelFrame.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "Resource/ObjectManager.h"
#include "Resource/Material.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"

#include <algorithm>
#include <cstring>

HS_NS_EDITOR_BEGIN

HierarchyPanel::HierarchyPanel(Window* window)
    : Panel(window, "Hierarchy")
{
}

bool HierarchyPanel::Setup()
{
    return true;
}

void HierarchyPanel::Cleanup()
{
}

void HierarchyPanel::Draw()
{
    if (!IsVisible())
    {
        return;
    }

    EditorPanelWindowOptions panelOptions{};
    panelOptions.pOpen = GetVisibilityBinding();
    EditorPanelFrame::BeginStandardPanel("Hierarchy", panelOptions);

    Scene* scene = EditorContext::Get().GetActiveScene();
    if (!scene)
    {
        EditorTreeWidgets::EmptyState("No active scene");
        EditorPanelFrame::EndStandardPanel();
        return;
    }

    // Search bar
    if (EditorTreeWidgets::SearchBar("##Search", "Search...", _searchBuffer, sizeof(_searchBuffer)))
    {
        // Search text changed
    }

    ImGui::Separator();

    // Scene header
    bool sceneOpen = EditorTreeWidgets::BeginNode(scene->GetName().c_str(), false, false, true);

    // Right-click on scene header for context menu
    if (ImGui::BeginPopupContextItem("SceneContextMenu"))
    {
        drawCreateEntityMenu(scene);
        ImGui::EndPopup();
    }

    if (sceneOpen)
    {
        // Traverse root entities
        auto& sceneGraph = scene->GetSceneGraph();
        const auto& roots = sceneGraph.GetRoots();

        for (entt::entity handle : roots)
        {
            if (scene->GetRegistry().valid(handle))
            {
                Entity entity = scene->GetEntity(handle);
                if (matchesSearch(entity))
                {
                    drawEntityNode(entity, 0);
                }
            }
        }

        ImGui::TreePop();
    }

    // Empty-space context menu for root creation
    drawContextMenu();

    // Drop target for ASSET_MODEL on empty area
    // Use an invisible dummy to cover remaining space as drop target
    ImVec2 remaining = ImGui::GetContentRegionAvail();
    if (remaining.y > 0)
    {
        ImGui::Dummy(remaining);
    }
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MODEL"))
        {
            std::string assetPath(static_cast<const char*>(payload->Data));

            if (scene)
            {
                hs::editor::AssetDatabase& assetDB = hs::editor::AssetDatabase::Get();
                hs::Model* model = assetDB.LoadModel(assetPath);
                if (model)
                {
                    // Extract display name from asset path
                    std::string entityName = assetPath;
                    size_t lastSlash = entityName.rfind('/');
                    if (lastSlash != std::string::npos)
                        entityName = entityName.substr(lastSlash + 1);
                    size_t dot = entityName.rfind('.');
                    if (dot != std::string::npos)
                        entityName = entityName.substr(0, dot);

                    Entity entity = scene->CreateEntity(entityName);
                    auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
                    initializeMeshRenderer(meshRenderer, model->GetMesh(), model->GetMaterial());

                    EditorContext::Get().SetSelectedEntity(entity);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Click on empty space to deselect
    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
    {
        EditorContext::Get().ClearSelection();
    }

    EditorPanelFrame::EndStandardPanel();
}

void HierarchyPanel::drawEntityNode(Entity entity, int depth)
{
    if (!entity.IsValid())
        return;

    Scene* scene = entity.GetScene();
    auto& registry = scene->GetRegistry();
    entt::entity handle = entity.GetHandle();

    // Get entity name
    std::string name = "Entity";
    if (entity.HasComponent<TagComponent>())
    {
        name = entity.GetComponent<TagComponent>().name;
    }

    // Get icon based on components
    const char* icon = getEntityIcon(entity);
    std::string displayName = std::string(icon) + " " + name;

    // Check if this entity is selected
    bool isSelected = (EditorContext::Get().GetSelectedEntity() == entity);

    // Determine tree node flags
    bool shouldOpenNode = _searchBuffer[0] != '\0' || hasSelectedDescendant(entity);

    // Check if entity has children
    bool hasChildren = false;
    if (entity.HasComponent<TransformComponent>())
    {
        hasChildren = entity.GetComponent<TransformComponent>().HasChildren();
    }

    // Push unique ID for this entity
    ImGui::PushID(static_cast<int>(handle));

    // If renaming this entity
    if (_renamingEntity == entity)
    {
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##Rename", _renameBuffer, sizeof(_renameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            if (entity.HasComponent<TagComponent>())
            {
                entity.GetComponent<TagComponent>().name = _renameBuffer;
            }
            _renamingEntity = Entity();
        }

        // Cancel on escape or lose focus
        if (ImGui::IsKeyPressed(ImGuiKey_Escape) ||
            (!ImGui::IsItemFocused() && !ImGui::IsItemActive()))
        {
            _renamingEntity = Entity();
        }

        ImGui::PopID();
        return;
    }

    if (shouldOpenNode)
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    }

    // Draw tree node
    bool nodeOpen = EditorTreeWidgets::BeginNode(
        displayName.c_str(),
        isSelected,
        !hasChildren,
        shouldOpenNode,
        ImGuiTreeNodeFlags_OpenOnDoubleClick);

    // Click to select
    if (EditorTreeWidgets::IsSelectionClick())
    {
        EditorContext::Get().SetSelectedEntity(entity);
    }

    // Double-click to rename
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
    {
        _renamingEntity = entity;
        if (entity.HasComponent<TagComponent>())
        {
            strncpy(_renameBuffer, entity.GetComponent<TagComponent>().name.c_str(), sizeof(_renameBuffer) - 1);
            _renameBuffer[sizeof(_renameBuffer) - 1] = '\0';
        }
    }

    // Right-click context menu
    if (ImGui::BeginPopupContextItem())
    {
        drawCreateEntityMenu(scene, entity);

        if (ImGui::MenuItem("Duplicate"))
        {
            // TODO: Implement entity duplication
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Rename"))
        {
            _renamingEntity = entity;
            if (entity.HasComponent<TagComponent>())
            {
                strncpy(_renameBuffer, entity.GetComponent<TagComponent>().name.c_str(), sizeof(_renameBuffer) - 1);
                _renameBuffer[sizeof(_renameBuffer) - 1] = '\0';
            }
        }

        if (ImGui::MenuItem("Delete"))
        {
            // Clear selection if deleting selected entity
            if (EditorContext::Get().GetSelectedEntity() == entity)
            {
                EditorContext::Get().ClearSelection();
            }
            scene->DestroyEntity(entity);
            ImGui::EndPopup();
            ImGui::PopID();
            if (nodeOpen)
                ImGui::TreePop();
            return;
        }

        ImGui::EndPopup();
    }

    // Drag source for reparenting
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload("ENTITY_HANDLE", &handle, sizeof(entt::entity));
        ImGui::Text("%s", name.c_str());
        ImGui::EndDragDropSource();
    }

    // Drop target for reparenting
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_HANDLE"))
        {
            entt::entity droppedHandle = *static_cast<const entt::entity*>(payload->Data);
            if (droppedHandle != handle)
            {
                scene->GetSceneGraph().SetParent(droppedHandle, handle);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Draw children if node is open
    if (nodeOpen)
    {
        if (entity.HasComponent<TransformComponent>())
        {
            const auto& transform = entity.GetComponent<TransformComponent>();
            for (entt::entity childHandle : transform.children)
            {
                if (registry.valid(childHandle))
                {
                    Entity child = scene->GetEntity(childHandle);
                    if (matchesSearch(child))
                    {
                        drawEntityNode(child, depth + 1);
                    }
                }
            }
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void HierarchyPanel::drawContextMenu()
{
    Scene* scene = EditorContext::Get().GetActiveScene();
    if (!scene)
    {
        return;
    }

    if (ImGui::BeginPopupContextWindow("HierarchyWindowContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        drawCreateEntityMenu(scene);
        ImGui::EndPopup();
    }
}

bool HierarchyPanel::drawCreateEntityMenu(Scene* scene, Entity parent)
{
    if (!scene)
    {
        return false;
    }

    bool created = false;
    if (ImGui::BeginMenu("Create..."))
    {
        if (ImGui::MenuItem("Empty Entity"))
        {
            created = createEmptyEntity(scene, parent).IsValid();
        }

        if (ImGui::MenuItem("Cube"))
        {
            created = createPrimitiveEntity(scene, PrimitiveType::Cube, parent).IsValid() || created;
        }

        if (ImGui::MenuItem("Sphere"))
        {
            created = createPrimitiveEntity(scene, PrimitiveType::Sphere, parent).IsValid() || created;
        }

        if (ImGui::MenuItem("Plane"))
        {
            created = createPrimitiveEntity(scene, PrimitiveType::Plane, parent).IsValid() || created;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Camera"))
        {
            created = createPrimitiveEntity(scene, PrimitiveType::Camera, parent).IsValid() || created;
        }

        if (ImGui::MenuItem("Directional Light"))
        {
            created = createPrimitiveEntity(scene, PrimitiveType::DirectionalLight, parent).IsValid() || created;
        }

        ImGui::EndMenu();
    }

    return created;
}

Entity HierarchyPanel::createEmptyEntity(Scene* scene, Entity parent)
{
    if (!scene)
    {
        return Entity();
    }

    Entity entity = parent.IsValid()
        ? scene->CreateChildEntity(parent, "Entity")
        : scene->CreateEntity("Entity");
    EditorContext::Get().SetSelectedEntity(entity);
    return entity;
}

Entity HierarchyPanel::createPrimitiveEntity(Scene* scene, PrimitiveType primitiveType, Entity parent)
{
    if (!scene)
    {
        return Entity();
    }

    const char* entityName = "Entity";
    const Mesh* fallbackMesh = nullptr;
    switch (primitiveType)
    {
    case PrimitiveType::Cube:
        entityName = "Cube";
        fallbackMesh = ObjectManager::GetFallbackMeshCube();
        break;
    case PrimitiveType::Sphere:
        entityName = "Sphere";
        fallbackMesh = ObjectManager::GetFallbackMeshSphere();
        break;
    case PrimitiveType::Plane:
        entityName = "Plane";
        fallbackMesh = ObjectManager::GetFallbackMeshPlane();
        break;
    case PrimitiveType::Camera:
        entityName = "Camera";
        break;
    case PrimitiveType::DirectionalLight:
        entityName = "Directional Light";
        break;
    }

    Entity entity = parent.IsValid()
        ? scene->CreateChildEntity(parent, entityName)
        : scene->CreateEntity(entityName);

    if (primitiveType == PrimitiveType::Camera)
    {
        auto& camera = entity.AddComponent<CameraComponent>();
        camera.isActive = true;
        camera.isPrimary = true;
        camera.priority = 100;
    }
    else if (primitiveType == PrimitiveType::DirectionalLight)
    {
        auto& light = entity.AddComponent<LightComponent>();
        light.type = ELightType::Directional;
        auto& transform = entity.GetComponent<TransformComponent>();
        transform.SetPosition(glm::vec3(0.0f, 3.0f, 0.0f));
        transform.SetEulerAngles(glm::vec3(45.0f, -45.0f, 0.0f));
    }
    else
    {
        auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
        initializeMeshRenderer(meshRenderer, const_cast<Mesh*>(fallbackMesh), createPrimitiveMaterial());
    }

    EditorContext::Get().SetSelectedEntity(entity);
    return entity;
}

void HierarchyPanel::initializeMeshRenderer(MeshRendererComponent& meshRenderer, Mesh* mesh, Material* material) const
{
    meshRenderer.mesh = mesh;
    meshRenderer.materials.clear();

    if (meshRenderer.mesh)
    {
        const auto& bound = meshRenderer.mesh->GetBound();
        meshRenderer.localBounds = AABB(glm::vec3(bound.min), glm::vec3(bound.max));
    }

    if (material)
    {
        meshRenderer.materials.push_back(material);
    }

    meshRenderer.boundsDirty = true;
}

Material* HierarchyPanel::createPrimitiveMaterial()
{
    Scoped<Material> material = MakeScoped<Material>();
    material->SetTexture(EMaterialTextureType::Diffuse, const_cast<Image*>(ObjectManager::GetFallbackImage2DWhite()));
    material->SetDiffuseColor(glm::vec4(1.0f));

    Material* materialPtr = material.get();
    _runtimeMaterials.push_back(std::move(material));
    return materialPtr;
}

const char* HierarchyPanel::getEntityIcon(Entity entity) const
{
    if (!entity.IsValid())
        return "";

    // Check components and return appropriate icon
    if (entity.HasComponent<CameraComponent>())
        return EditorIcons::Camera;

    if (entity.HasComponent<LightComponent>())
        return EditorIcons::LightMode;

    if (entity.HasComponent<MeshRendererComponent>())
        return EditorIcons::ViewInAr;

    // Default icon
    return EditorIcons::Draft;
}

bool HierarchyPanel::matchesSearch(Entity entity) const
{
    // If no search text, show everything
    if (_searchBuffer[0] == '\0')
        return true;

    if (!entity.IsValid())
        return false;

    // Check name
    if (entity.HasComponent<TagComponent>())
    {
        const std::string& name = entity.GetComponent<TagComponent>().name;
        // Case-insensitive search
        std::string lowerName = name;
        std::string lowerSearch = _searchBuffer;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

        if (lowerName.find(lowerSearch) != std::string::npos)
            return true;
    }

    // If this entity doesn't match, check if any child matches
    if (entity.HasComponent<TransformComponent>())
    {
        const auto& transform = entity.GetComponent<TransformComponent>();
        Scene* scene = entity.GetScene();

        for (entt::entity childHandle : transform.children)
        {
            if (scene->GetRegistry().valid(childHandle))
            {
                Entity child = scene->GetEntity(childHandle);
                if (matchesSearch(child))
                    return true;
            }
        }
    }

    return false;
}

bool HierarchyPanel::hasSelectedDescendant(Entity entity) const
{
    Entity selectedEntity = EditorContext::Get().GetSelectedEntity();
    if (!entity.IsValid() || !selectedEntity.IsValid())
    {
        return false;
    }
    if (entity.GetScene() != selectedEntity.GetScene())
    {
        return false;
    }
    if (entity == selectedEntity)
    {
        return false;
    }

    return entity.GetScene()->GetSceneGraph().IsAncestorOf(entity.GetHandle(), selectedEntity.GetHandle());
}

void HierarchyPanel::handleDragDrop(Entity entity)
{
    // Handled inline in drawEntityNode
}

HS_NS_EDITOR_END
