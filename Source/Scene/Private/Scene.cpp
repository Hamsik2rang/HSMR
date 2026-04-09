//
//  Scene.cpp
//  HSMR
//

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Systems/TransformSystem.h"
#include "Scene/Systems/MeshBoundsSystem.h"
#include "Core/Log.h"

HS_NS_BEGIN

namespace
{
bool isCameraPreferredForGameView(
    entt::entity lhsEntity,
    const CameraComponent& lhs,
    entt::entity rhsEntity,
    const CameraComponent& rhs)
{
    if (lhs.isActive != rhs.isActive)
    {
        return lhs.isActive && !rhs.isActive;
    }

    if (lhs.priority != rhs.priority)
    {
        return lhs.priority > rhs.priority;
    }

    if (lhs.isPrimary != rhs.isPrimary)
    {
        return lhs.isPrimary && !rhs.isPrimary;
    }

    return entt::to_integral(lhsEntity) < entt::to_integral(rhsEntity);
}
}

Scene::Scene(const std::string& name)
    : _name(name)
    , _sceneGraph(_registry)
{
    AddSystem<TransformSystem>(_sceneGraph);
    AddSystem<MeshBoundsSystem>();
    HS_LOG(info, "[Scene] Created scene: {}", _name.c_str());
}

Scene::~Scene()
{
    Clear();
    HS_LOG(info, "[Scene] Destroyed scene: {}", _name.c_str());
}

Entity Scene::CreateEntity(const std::string& name)
{
    entt::entity handle = _registry.create();

    // 기본 컴포넌트 추가
    _registry.emplace<TagComponent>(handle, name);
    _registry.emplace<TransformComponent>(handle);

    // SceneGraph에 루트로 추가
    _sceneGraph.AddRoot(handle);

    return Entity(handle, this);
}

Entity Scene::CreateChildEntity(Entity parent, const std::string& name)
{
    if (!parent.IsValid())
    {
        HS_LOG(warning, "[Scene] Invalid parent entity, creating as root");
        return CreateEntity(name);
    }

    entt::entity handle = _registry.create();

    // 기본 컴포넌트 추가
    _registry.emplace<TagComponent>(handle, name);
    _registry.emplace<TransformComponent>(handle);

    // 부모 설정
    _sceneGraph.SetParent(handle, parent.GetHandle());

    return Entity(handle, this);
}

void Scene::DestroyEntity(Entity entity)
{
    if (!entity.IsValid())
    {
        return;
    }
    DestroyEntity(entity.GetHandle());
}

void Scene::DestroyEntity(entt::entity handle)
{
    if (!_registry.valid(handle))
    {
        return;
    }

    destroyEntityRecursive(handle);
}

void Scene::destroyEntityRecursive(entt::entity entity)
{
    if (!_registry.valid(entity))
    {
        return;
    }

    // 자식들 먼저 삭제
    if (_registry.all_of<TransformComponent>(entity))
    {
        auto& transform = _registry.get<TransformComponent>(entity);

        // 복사본으로 순회 (삭제 중 수정 방지)
        std::vector<entt::entity> children = transform.children;
        for (auto child : children)
        {
            destroyEntityRecursive(child);
        }

        // 부모에서 제거
        if (transform.parent != entt::null && _registry.valid(transform.parent))
        {
            if (_registry.all_of<TransformComponent>(transform.parent))
            {
                auto& parentTransform = _registry.get<TransformComponent>(transform.parent);
                auto it = std::find(parentTransform.children.begin(),
                                    parentTransform.children.end(), entity);
                if (it != parentTransform.children.end())
                {
                    parentTransform.children.erase(it);
                }
            }
        }
    }

    // 루트 목록에서 제거
    _sceneGraph.RemoveRoot(entity);

    // Entity 삭제
    _registry.destroy(entity);
}

Entity Scene::FindEntityByName(const std::string& name)
{
    auto view = _registry.view<TagComponent>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.name == name)
        {
            return Entity(entity, this);
        }
    }
    return Entity();
}

std::vector<Entity> Scene::FindEntitiesByLayer(uint32 layer)
{
    std::vector<Entity> result;

    auto view = _registry.view<TagComponent>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.layer == layer)
        {
            result.emplace_back(entity, this);
        }
    }

    return result;
}

std::vector<Entity> Scene::FindStaticEntities()
{
    std::vector<Entity> result;

    auto view = _registry.view<TagComponent>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.isStatic)
        {
            result.emplace_back(entity, this);
        }
    }

    return result;
}

void Scene::Update(float deltaTime)
{
    for (auto& system : _systems)
    {
        system->Update(_registry, deltaTime);
    }
}

size_t Scene::GetEntityCount() const
{
    return _registry.storage<entt::entity>()->size();
}

size_t Scene::GetActiveEntityCount() const
{
    size_t count = 0;
    auto view = _registry.view<TagComponent>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.isActive)
        {
            count++;
        }
    }
    return count;
}

Entity Scene::GetPrimaryCamera()
{
    auto view = _registry.view<CameraComponent>();
    for (auto [entity, camera] : view.each())
    {
        if (camera.isPrimary)
        {
            return Entity(entity, this);
        }
    }
    return Entity();
}

Entity Scene::GetBestGameCamera()
{
    Entity bestCamera;
    auto view = _registry.view<CameraComponent>();
    for (auto [entity, camera] : view.each())
    {
        if (!bestCamera.IsValid())
        {
            bestCamera = Entity(entity, this);
            continue;
        }

        const CameraComponent& currentBest = bestCamera.GetComponent<CameraComponent>();
        if (isCameraPreferredForGameView(entity, camera, bestCamera.GetHandle(), currentBest))
        {
            bestCamera = Entity(entity, this);
        }
    }

    if (bestCamera.IsValid() && !bestCamera.GetComponent<CameraComponent>().isActive)
    {
        return Entity();
    }

    return bestCamera;
}

void Scene::SetPrimaryCamera(Entity camera)
{
    if (!camera.IsValid() || !camera.HasComponent<CameraComponent>())
    {
        HS_LOG(warning, "[Scene] Invalid camera entity");
        return;
    }

    // 기존 primary 해제
    auto view = _registry.view<CameraComponent>();
    for (auto [entity, cam] : view.each())
    {
        cam.isPrimary = false;
    }

    // 새 primary 설정
    camera.GetComponent<CameraComponent>().isPrimary = true;
}

void Scene::Clear()
{
    // 모든 루트에서 시작하여 삭제
    std::vector<entt::entity> roots = _sceneGraph.GetRoots();
    for (auto root : roots)
    {
        destroyEntityRecursive(root);
    }

    _registry.clear();
}

Entity Scene::GetEntity(entt::entity handle)
{
    if (_registry.valid(handle))
    {
        return Entity(handle, this);
    }
    return Entity();
}

HS_NS_END
