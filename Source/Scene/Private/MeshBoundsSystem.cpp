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
        if (meshRenderer.boundsDirty || meshRenderer.boundsWorldVersion != transform.worldVersion)
        {
            meshRenderer.UpdateWorldBounds(transform.worldMatrix);
            meshRenderer.boundsWorldVersion = transform.worldVersion;
        }
    }
}

HS_NS_END
