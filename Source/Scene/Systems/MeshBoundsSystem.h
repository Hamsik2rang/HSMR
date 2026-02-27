//
//  MeshBoundsSystem.h
//  HSMR
//

#ifndef __HS_SCENE_MESHBOUNDSSYSTEM_H__
#define __HS_SCENE_MESHBOUNDSSYSTEM_H__

#include "Scene/Systems/IComponentSystem.h"

HS_NS_BEGIN

class HS_SCENE_API MeshBoundsSystem : public IComponentSystem
{
public:
    HS_GENERATE_REFLECTION(MeshBoundsSystem)

    void Update(entt::registry& registry, float deltaTime) override;
};

HS_NS_END

#endif
