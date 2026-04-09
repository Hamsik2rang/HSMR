//
//  HierarchyPanel.h
//  Editor
//
//  Entity hierarchy tree view with selection support
//

#ifndef __HS_HIERARCHY_PANEL_H__
#define __HS_HIERARCHY_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"
#include "Scene/Entity.h"
#include "Scene/Components/MeshRendererComponent.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"

#include <string>
#include <vector>

HS_NS_EDITOR_BEGIN

class HierarchyPanel : public Panel
{
public:
    HierarchyPanel(Window* window);
    ~HierarchyPanel() override = default;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

private:
    enum class PrimitiveType : uint8
    {
        Cube,
        Sphere,
        Plane,
        Camera,
        DirectionalLight,
    };

    // Entity tree node rendering
    void drawEntityNode(Entity entity, int depth = 0);

    // Context menu for entity operations
    void drawContextMenu();
    bool drawCreateEntityMenu(Scene* scene, Entity parent = Entity());
    Entity createEmptyEntity(Scene* scene, Entity parent);
    Entity createPrimitiveEntity(Scene* scene, PrimitiveType primitiveType, Entity parent);
    void initializeMeshRenderer(MeshRendererComponent& meshRenderer, Mesh* mesh, Material* material) const;
    Material* createPrimitiveMaterial();

    // Get icon for entity based on components
    const char* getEntityIcon(Entity entity) const;

    // Search/filter
    bool matchesSearch(Entity entity) const;
    bool hasSelectedDescendant(Entity entity) const;

    // Drag & drop for reparenting
    void handleDragDrop(Entity entity);

    // State
    char _searchBuffer[256] = "";
    std::vector<Scoped<Material>> _runtimeMaterials;

    // Rename state
    Entity _renamingEntity;
    char _renameBuffer[256] = "";
};

HS_NS_EDITOR_END

#endif
