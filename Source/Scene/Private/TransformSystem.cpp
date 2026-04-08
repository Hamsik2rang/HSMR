//
//  TransformSystem.cpp
//  HSMR
//

#include "Scene/Systems/TransformSystem.h"
#include "Scene/SceneGraph.h"
#include "Scene/Components/TransformComponent.h"

HS_NS_BEGIN

TransformSystem::TransformSystem(SceneGraph& sceneGraph)
    : _sceneGraph(sceneGraph)
{
}

void TransformSystem::Update(entt::registry& registry, float deltaTime)
{
    const auto& roots = _sceneGraph.GetRoots();
    for (auto root : roots)
    {
        if (registry.valid(root) && registry.all_of<TransformComponent>(root))
        {
            updateWorldTransformRecursive(registry, root, glm::mat4(1.0f), false);
        }
    }
}

void TransformSystem::updateWorldTransformRecursive(entt::registry& registry,
                                                     entt::entity entity,
                                                     const glm::mat4& parentWorld,
                                                     bool parentDirty)
{
    if (!registry.valid(entity) || !registry.all_of<TransformComponent>(entity))
    {
        return;
    }

    auto& transform = registry.get<TransformComponent>(entity);

    bool shouldUpdate = parentDirty || transform.isDirty || transform.worldDirty;
    if (shouldUpdate)
    {
        glm::mat4 localMatrix = transform.GetLocalMatrix();
        transform.worldMatrix = parentWorld * localMatrix;
        transform.isDirty = false;
        transform.worldDirty = false;
        transform.worldVersion++;
    }

    for (auto child : transform.children)
    {
        updateWorldTransformRecursive(registry, child, transform.worldMatrix, shouldUpdate);
    }
}

HS_NS_END
