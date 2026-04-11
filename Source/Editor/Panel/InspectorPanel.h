//
//  InspectorPanel.h
//  Editor
//
//  Component editor panel for selected entity
//

#ifndef __HS_INSPECTOR_PANEL_H__
#define __HS_INSPECTOR_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/Panel.h"
#include "Scene/Entity.h"
#include "Scene/Components/MeshRendererComponent.h"
#include "Resource/Material.h"

#include <vector>

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API InspectorPanel : public Panel
{
public:
    InspectorPanel(Window* window, const char* panelId = "Inspector");
    ~InspectorPanel() override;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

protected:
    // Helper widgets
    bool drawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f, float speed = 0.1f);
    void drawColorEdit(const char* label, glm::vec3& color);

private:
    // Component editors
    void drawTagComponent(Entity entity);
    void drawTransformComponent(Entity entity);
    void drawMeshRendererComponent(Entity entity);
    void drawCameraComponent(Entity entity);
    void drawLightComponent(Entity entity);
    void drawMaterialSlotEditor(MeshRendererComponent& meshRenderer, uint32 slotIndex);
    bool drawMaterialEditor(Material* material, const char* idSuffix);
    bool drawMaterialTextureSlot(Material* material, EMaterialTextureType type, const char* label, const char* idSuffix);
    bool drawMaterialParameterBlockEditor(Material* material, const char* idSuffix);
    void drawMaterialAssetInspector(const std::string& assetPath);
    Material* createRuntimeMaterial();
    Material* createMaterialAsset(const std::string& suggestedName, uint32 slotIndex);
    bool saveMaterialAs(Material* material, const std::string& suggestedName);
    void persistMaterialIfAssetBacked(Material* material);


    // Add component popup
    void drawAddComponentButton(Entity entity);

    // Component header with remove button
    template<typename T>
    bool drawComponentHeader(const char* name, Entity entity, bool canRemove = true);

    std::vector<Scoped<Material>> _runtimeMaterials;
};

HS_NS_EDITOR_END

#endif /* __HS_INSPECTOR_PANEL_H__ */
