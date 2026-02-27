//
//  IComponentSystem.h
//  HSMR
//

#ifndef __HS_SCENE_ICOMPONENTSYSTEM_H__
#define __HS_SCENE_ICOMPONENTSYSTEM_H__

#include "Precompile.h"
#include "Core/TypeId.h"
#include <entt/entt.hpp>

HS_NS_BEGIN

class HS_SCENE_API IComponentSystem
{
public:
    virtual ~IComponentSystem() = default;

    virtual void Update(entt::registry& registry, float deltaTime) = 0;
    virtual const char* GetTypeName() const = 0;
    virtual TypeId GetTypeId() const = 0;
};

HS_NS_END

#endif
