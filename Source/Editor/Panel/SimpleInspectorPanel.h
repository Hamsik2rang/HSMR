#ifndef __HS_SIMPLE_INSPECTOR_PANEL_H__
#define __HS_SIMPLE_INSPECTOR_PANEL_H__

#include "Precompile.h"

#include "Editor/Panel/InspectorPanel.h"

#include "Scene/Entity.h"
#include "Scene/Components/LightComponent.h"
#include "Scene/Components/TransformComponent.h"

HS_NS_EDITOR_BEGIN

class SimpleInspectorPanel : public InspectorPanel
{
public:
    SimpleInspectorPanel(Window* window)
        : InspectorPanel(window, "Control Panel") {};
    ~SimpleInspectorPanel() override = default;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

    void SetMainLight(Entity lightEntity) { HS_ASSERT(lightEntity.IsValid(),"Invalid Entity!"); _mainLightEntity = lightEntity; }
    void SetMainCamera(Entity cameraEntity){ HS_ASSERT(cameraEntity.IsValid(),"Invalid Entity!"); _mainCameraEntity = cameraEntity; }
    void SetTarget(Entity entity) { HS_ASSERT(entity.IsValid(), "Invalid Entity!"); _targetEntity = entity; }

private:
    Entity _targetEntity;
    Entity _mainLightEntity;
    Entity _mainCameraEntity;
};

HS_NS_EDITOR_END

#endif
