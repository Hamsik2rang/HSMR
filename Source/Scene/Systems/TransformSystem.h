//
//  TransformSystem.h
//  HSMR
//

#ifndef __HS_SCENE_TRANSFORMSYSTEM_H__
#define __HS_SCENE_TRANSFORMSYSTEM_H__

#include "Scene/Systems/IComponentSystem.h"
#include "Core/Math/Common.h"

HS_NS_BEGIN

class SceneGraph;

class HS_SCENE_API TransformSystem : public IComponentSystem
{
public:
    TransformSystem(SceneGraph& sceneGraph);

    HS_GENERATE_REFLECTION(TransformSystem)

    void Update(entt::registry& registry, float deltaTime) override;

private:
    void updateWorldTransformRecursive(entt::registry& registry,
                                       entt::entity entity,
                                       const glm::mat4& parentWorld);
    SceneGraph& _sceneGraph;
};

HS_NS_END

#endif
