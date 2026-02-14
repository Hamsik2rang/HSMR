//
//  Entity.cpp
//  HSMR
//

#include "Scene/Entity.h"
#include "Scene/Scene.h"

HS_NS_BEGIN

Entity::Entity(entt::entity handle, Scene* scene)
    : _handle(handle)
    , _scene(scene)
{
}

bool Entity::IsValid() const
{
    return _scene != nullptr &&
           _handle != entt::null &&
           _scene->GetRegistry().valid(_handle);
}

HS_NS_END
