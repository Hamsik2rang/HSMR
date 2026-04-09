//
//  Scene.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include "Scene/SceneGraph.h"
#include "Scene/Systems/IComponentSystem.h"
#include "Scene/Components/Components.h"
#include <entt/entt.hpp>
#include <string>
#include <vector>

HS_NS_BEGIN

class Entity;

/**
 * @brief 씬 클래스
 *
 * EnTT registry와 SceneGraph를 통합하여 관리합니다.
 * Entity 생성/삭제, 컴포넌트 쿼리 등의 인터페이스를 제공합니다.
 */
class HS_SCENE_API Scene
{
public:
    Scene(const std::string& name = "Untitled");
    ~Scene();

    // 복사 금지
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // 이동 허용
    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;

    // ===== Entity 생성/삭제 =====

    /**
     * @brief Entity 생성
     * @param name Entity 이름
     * @return 생성된 Entity
     */
    Entity CreateEntity(const std::string& name = "Entity");

    /**
     * @brief 자식 Entity 생성
     * @param parent 부모 Entity
     * @param name Entity 이름
     * @return 생성된 Entity
     */
    Entity CreateChildEntity(Entity parent, const std::string& name = "Entity");

    /**
     * @brief Entity 삭제 (자손 포함)
     * @param entity 삭제할 Entity
     */
    void DestroyEntity(Entity entity);

    /**
     * @brief Entity 삭제 (handle로)
     */
    void DestroyEntity(entt::entity handle);

    // ===== Entity 검색 =====

    /**
     * @brief 이름으로 Entity 검색
     * @param name 검색할 이름
     * @return 찾은 Entity (없으면 invalid)
     */
    Entity FindEntityByName(const std::string& name);

    /**
     * @brief 태그로 Entity 검색
     * @param tag 검색할 태그
     * @return 찾은 Entity 목록
     */
    std::vector<Entity> FindEntitiesByLayer(uint32 layer);

    /**
     * @brief 모든 static Entity 검색
     */
    std::vector<Entity> FindStaticEntities();

    // ===== Component 쿼리 =====

    /**
     * @brief 특정 컴포넌트를 가진 Entity들의 view 반환
     */
    template<typename... Components>
    auto View()
    {
        return _registry.view<Components...>();
    }

    template<typename... Components>
    auto View() const
    {
        return _registry.view<Components...>();
    }

    /**
     * @brief 특정 컴포넌트를 가진 Entity 개수
     */
    template<typename T>
    size_t Count() const
    {
        return _registry.view<T>().size();
    }

    // ===== Scene 업데이트 =====

    /**
     * @brief 매 프레임 호출
     * - Transform 전파
     * - 기타 업데이트 로직
     */
    void Update(float deltaTime);

    // ===== System 관리 =====

    template<typename T, typename... Args>
    T* AddSystem(Args&&... args);

    template<typename T>
    T* GetSystem();

    // ===== SceneGraph 접근 =====

    SceneGraph& GetSceneGraph() { return _sceneGraph; }
    const SceneGraph& GetSceneGraph() const { return _sceneGraph; }

    // ===== Registry 접근 =====

    entt::registry& GetRegistry() { return _registry; }
    const entt::registry& GetRegistry() const { return _registry; }

    // ===== Scene 정보 =====

    const std::string& GetName() const { return _name; }
    void SetName(const std::string& name) { _name = name; }

    /**
     * @brief 전체 Entity 개수
     */
    size_t GetEntityCount() const;

    /**
     * @brief 활성화된 Entity 개수
     */
    size_t GetActiveEntityCount() const;

    // ===== 카메라 =====

    /**
     * @brief Primary 카메라 Entity 반환
     */
    Entity GetPrimaryCamera();
    Entity GetBestGameCamera();

    /**
     * @brief Primary 카메라 설정
     */
    void SetPrimaryCamera(Entity camera);

    // ===== 유틸리티 =====

    /**
     * @brief 모든 Entity 삭제
     */
    void Clear();

    /**
     * @brief Entity handle로 Entity 객체 생성
     */
    Entity GetEntity(entt::entity handle);

private:
    void destroyEntityRecursive(entt::entity entity);

    std::string _name;
    entt::registry _registry;
    SceneGraph _sceneGraph;
    std::vector<Scoped<IComponentSystem>> _systems;
};

// ===== Template Implementations =====

template<typename T, typename... Args>
T* Scene::AddSystem(Args&&... args)
{
    auto system = MakeScoped<T>(std::forward<Args>(args)...);
    T* ptr = system.get();
    _systems.push_back(std::move(system));
    return ptr;
}

template<typename T>
T* Scene::GetSystem()
{
    TypeId targetId = T::GetStaticTypeId();
    for (auto& system : _systems)
    {
        if (system->GetTypeId() == targetId)
        {
            return static_cast<T*>(system.get());
        }
    }
    return nullptr;
}

HS_NS_END

// Entity 구현을 위해 여기서 include
#include "Scene/Entity.h"
