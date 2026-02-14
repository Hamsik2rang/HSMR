//
//  SceneGraph.h
//  HSMR
//

#pragma once

#include "Precompile.h"
#include "Scene/Components/TransformComponent.h"
#include <entt/entt.hpp>
#include <vector>
#include <queue>
#include <functional>

HS_NS_BEGIN

/**
 * @brief 씬 그래프 (계층 구조 관리)
 *
 * Entity들의 부모-자식 관계를 관리하고,
 * Transform 전파를 수행합니다.
 */
class HS_SCENE_API SceneGraph
{
public:
    SceneGraph(entt::registry& registry);
    ~SceneGraph() = default;

    // ===== 계층 관계 설정 =====

    /**
     * @brief 부모 설정
     * @param child 자식 entity
     * @param parent 부모 entity (entt::null이면 루트로 설정)
     */
    void SetParent(entt::entity child, entt::entity parent);

    /**
     * @brief 부모 관계 해제 (루트로 이동)
     */
    void RemoveParent(entt::entity child);

    /**
     * @brief 자식 추가
     */
    void AddChild(entt::entity parent, entt::entity child);

    /**
     * @brief 자식 제거
     */
    void RemoveChild(entt::entity parent, entt::entity child);

    // ===== 계층 쿼리 =====

    entt::entity GetParent(entt::entity entity) const;
    const std::vector<entt::entity>& GetChildren(entt::entity entity) const;
    std::vector<entt::entity> GetDescendants(entt::entity entity) const;
    std::vector<entt::entity> GetAncestors(entt::entity entity) const;
    entt::entity GetRoot(entt::entity entity) const;
    bool IsAncestorOf(entt::entity ancestor, entt::entity descendant) const;
    bool IsDescendantOf(entt::entity descendant, entt::entity ancestor) const;

    /**
     * @brief 모든 루트 entity 반환
     */
    const std::vector<entt::entity>& GetRoots() const { return _roots; }

    // ===== Transform 전파 =====

    /**
     * @brief 모든 dirty transform 업데이트
     */
    void UpdateWorldTransforms();

    /**
     * @brief 특정 entity와 그 자손들을 dirty로 표시
     */
    void MarkDirty(entt::entity entity);

    // ===== 계층 순회 =====

    /**
     * @brief 깊이 우선 순회
     * @param root 시작 entity (entt::null이면 모든 루트)
     * @param func 각 entity에 호출할 함수
     */
    template<typename Func>
    void TraverseDepthFirst(entt::entity root, Func&& func);

    /**
     * @brief 너비 우선 순회
     */
    template<typename Func>
    void TraverseBreadthFirst(entt::entity root, Func&& func);

    /**
     * @brief 모든 entity 순회 (모든 루트에서 시작)
     */
    template<typename Func>
    void TraverseAll(Func&& func);

    // ===== 루트 관리 =====

    void AddRoot(entt::entity entity);
    void RemoveRoot(entt::entity entity);

private:
    void updateWorldTransformRecursive(entt::entity entity, const glm::mat4& parentWorld);
    void markDirtyRecursive(entt::entity entity);

    entt::registry& _registry;
    std::vector<entt::entity> _roots;
    static const std::vector<entt::entity> _emptyChildren;
};

// ===== Template Implementations =====

template<typename Func>
void SceneGraph::TraverseDepthFirst(entt::entity root, Func&& func)
{
    if (root == entt::null)
    {
        // 모든 루트에서 시작
        for (auto rootEntity : _roots)
        {
            TraverseDepthFirst(rootEntity, std::forward<Func>(func));
        }
        return;
    }

    // 현재 노드 처리
    func(root);

    // 자식들 재귀 처리
    if (_registry.valid(root) && _registry.all_of<TransformComponent>(root))
    {
        const auto& transform = _registry.get<TransformComponent>(root);
        for (auto child : transform.children)
        {
            TraverseDepthFirst(child, std::forward<Func>(func));
        }
    }
}

template<typename Func>
void SceneGraph::TraverseBreadthFirst(entt::entity root, Func&& func)
{
    std::queue<entt::entity> queue;

    if (root == entt::null)
    {
        for (auto rootEntity : _roots)
        {
            queue.push(rootEntity);
        }
    }
    else
    {
        queue.push(root);
    }

    while (!queue.empty())
    {
        entt::entity current = queue.front();
        queue.pop();

        func(current);

        if (_registry.valid(current) && _registry.all_of<TransformComponent>(current))
        {
            const auto& transform = _registry.get<TransformComponent>(current);
            for (auto child : transform.children)
            {
                queue.push(child);
            }
        }
    }
}

template<typename Func>
void SceneGraph::TraverseAll(Func&& func)
{
    TraverseDepthFirst(entt::null, std::forward<Func>(func));
}

HS_NS_END
