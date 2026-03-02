//
//  Entity.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include "Core/Log.h"
#include <entt/entt.hpp>

HS_NS_BEGIN

class Scene;

/**
 * @brief EnTT entity를 감싸는 편의 클래스
 *
 * EnTT의 raw entity handle과 registry 접근을 캡슐화하여
 * 더 직관적인 API를 제공합니다.
 */
class HS_SCENE_API Entity
{
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene);
    Entity(const Entity&) = default;
    Entity& operator=(const Entity&) = default;

    /**
     * @brief 컴포넌트 추가
     * @tparam T 컴포넌트 타입
     * @tparam Args 생성자 인자 타입들
     * @param args 컴포넌트 생성자에 전달할 인자들
     * @return 추가된 컴포넌트 참조
     */
    template<typename T, typename... Args>
    T& AddComponent(Args&&... args);

    /**
     * @brief 컴포넌트 가져오기
     * @tparam T 컴포넌트 타입
     * @return 컴포넌트 참조
     */
    template<typename T>
    T& GetComponent();

    template<typename T>
    const T& GetComponent() const;

    /**
     * @brief 컴포넌트 소유 여부 확인
     * @tparam T 컴포넌트 타입
     * @return 컴포넌트 보유 시 true
     */
    template<typename T>
    bool HasComponent() const;

    /**
     * @brief 컴포넌트 제거
     * @tparam T 컴포넌트 타입
     */
    template<typename T>
    void RemoveComponent();

    /**
     * @brief 컴포넌트가 있으면 가져오고, 없으면 추가
     * @tparam T 컴포넌트 타입
     * @tparam Args 생성자 인자 타입들
     * @param args 컴포넌트 생성자에 전달할 인자들
     * @return 컴포넌트 참조
     */
    template<typename T, typename... Args>
    T& GetOrAddComponent(Args&&... args);

    /**
     * @brief Entity 유효성 검사
     * @return 유효한 entity면 true
     */
    bool IsValid() const;

    /**
     * @brief EnTT entity handle 반환
     */
    entt::entity GetHandle() const { return _handle; }

    /**
     * @brief 소속 Scene 반환
     */
    Scene* GetScene() const { return _scene; }

    bool operator==(const Entity& other) const
    {
        return _handle == other._handle && _scene == other._scene;
    }

    bool operator!=(const Entity& other) const
    {
        return !(*this == other);
    }

    operator bool() const { return IsValid(); }
    operator entt::entity() const { return _handle; }
    
    static const Entity Invalid() { return Entity(entt::null, nullptr); }

private:
    entt::entity _handle{ entt::null };
    Scene* _scene = nullptr;
};

HS_NS_END

// Template implementations
#include "Scene/Scene.h"

HS_NS_BEGIN

template<typename T, typename... Args>
T& Entity::AddComponent(Args&&... args)
{
    HS_ASSERT(!HasComponent<T>(), "Entity already has this component!");
    return _scene->GetRegistry().emplace<T>(_handle, std::forward<Args>(args)...);
}

template<typename T>
T& Entity::GetComponent()
{
    HS_ASSERT(HasComponent<T>(), "Entity does not have this component!");
    return _scene->GetRegistry().get<T>(_handle);
}

template<typename T>
const T& Entity::GetComponent() const
{
    HS_ASSERT(HasComponent<T>(), "Entity does not have this component!");
    return _scene->GetRegistry().get<T>(_handle);
}

template<typename T>
bool Entity::HasComponent() const
{
    return _scene->GetRegistry().all_of<T>(_handle);
}

template<typename T>
void Entity::RemoveComponent()
{
    HS_ASSERT(HasComponent<T>(), "Entity does not have this component!");
    _scene->GetRegistry().remove<T>(_handle);
}

template<typename T, typename... Args>
T& Entity::GetOrAddComponent(Args&&... args)
{
    if (HasComponent<T>())
    {
        return GetComponent<T>();
    }
    return AddComponent<T>(std::forward<Args>(args)...);
}

HS_NS_END
