//
//  MeshBoundsSystem.cpp
//  HSMR
//

#include "Scene/Systems/MeshBoundsSystem.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/MeshRendererComponent.h"

HS_NS_BEGIN

void MeshBoundsSystem::Update(entt::registry& registry, float deltaTime)
{
    auto view = registry.view<TransformComponent, MeshRendererComponent>();
    for (auto [entity, transform, meshRenderer] : view.each())
    {
        meshRenderer.UpdateWorldBounds(transform.worldMatrix);
    }
}

HS_NS_END
