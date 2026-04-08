//
//  SceneGraph.cpp
//  HSMR
//

#include "Scene/SceneGraph.h"
#include "Core/Log.h"
#include <algorithm>

HS_NS_BEGIN

const std::vector<entt::entity> SceneGraph::_emptyChildren;

SceneGraph::SceneGraph(entt::registry& registry)
    : _registry(registry)
{
}

void SceneGraph::SetParent(entt::entity child, entt::entity parent)
{
    if (!_registry.valid(child))
    {
        HS_LOG(warning, "[SceneGraph] Invalid child entity");
        return;
    }

    if (!_registry.all_of<TransformComponent>(child))
    {
        HS_LOG(warning, "[SceneGraph] Child entity has no TransformComponent");
        return;
    }

    auto& childTransform = _registry.get<TransformComponent>(child);
    entt::entity oldParent = childTransform.parent;

    // 같은 부모면 무시
    if (oldParent == parent)
    {
        return;
    }

    // 순환 참조 체크
    if (parent != entt::null && IsDescendantOf(parent, child))
    {
        HS_LOG(warning, "[SceneGraph] Cannot set parent: would create cycle");
        return;
    }

    // 이전 부모에서 제거
    if (oldParent != entt::null && _registry.valid(oldParent))
    {
        if (_registry.all_of<TransformComponent>(oldParent))
        {
            auto& oldParentTransform = _registry.get<TransformComponent>(oldParent);
            auto it = std::find(oldParentTransform.children.begin(),
                                oldParentTransform.children.end(), child);
            if (it != oldParentTransform.children.end())
            {
                oldParentTransform.children.erase(it);
            }
        }
    }
    else
    {
        // 루트 목록에서 제거
        RemoveRoot(child);
    }

    // 새 부모 설정
    childTransform.parent = parent;

    if (parent != entt::null)
    {
        if (_registry.valid(parent) && _registry.all_of<TransformComponent>(parent))
        {
            auto& parentTransform = _registry.get<TransformComponent>(parent);
            parentTransform.children.push_back(child);
        }
    }
    else
    {
        // 루트로 설정
        AddRoot(child);
    }

    // Dirty 표시
    if (_registry.all_of<TransformComponent>(child))
    {
        _registry.get<TransformComponent>(child).MarkDirty();
    }
}

void SceneGraph::RemoveParent(entt::entity child)
{
    SetParent(child, entt::null);
}

void SceneGraph::AddChild(entt::entity parent, entt::entity child)
{
    SetParent(child, parent);
}

void SceneGraph::RemoveChild(entt::entity parent, entt::entity child)
{
    if (!_registry.valid(parent) || !_registry.valid(child))
    {
        return;
    }

    if (!_registry.all_of<TransformComponent>(child))
    {
        return;
    }

    auto& childTransform = _registry.get<TransformComponent>(child);
    if (childTransform.parent == parent)
    {
        SetParent(child, entt::null);
    }
}

entt::entity SceneGraph::GetParent(entt::entity entity) const
{
    if (!_registry.valid(entity) || !_registry.all_of<TransformComponent>(entity))
    {
        return entt::null;
    }

    return _registry.get<TransformComponent>(entity).parent;
}

const std::vector<entt::entity>& SceneGraph::GetChildren(entt::entity entity) const
{
    if (!_registry.valid(entity) || !_registry.all_of<TransformComponent>(entity))
    {
        return _emptyChildren;
    }

    return _registry.get<TransformComponent>(entity).children;
}

std::vector<entt::entity> SceneGraph::GetDescendants(entt::entity entity) const
{
    std::vector<entt::entity> descendants;

    if (!_registry.valid(entity) || !_registry.all_of<TransformComponent>(entity))
    {
        return descendants;
    }

    std::queue<entt::entity> queue;
    const auto& transform = _registry.get<TransformComponent>(entity);
    for (auto child : transform.children)
    {
        queue.push(child);
    }

    while (!queue.empty())
    {
        entt::entity current = queue.front();
        queue.pop();

        descendants.push_back(current);

        if (_registry.valid(current) && _registry.all_of<TransformComponent>(current))
        {
            const auto& currentTransform = _registry.get<TransformComponent>(current);
            for (auto child : currentTransform.children)
            {
                queue.push(child);
            }
        }
    }

    return descendants;
}

std::vector<entt::entity> SceneGraph::GetAncestors(entt::entity entity) const
{
    std::vector<entt::entity> ancestors;

    if (!_registry.valid(entity) || !_registry.all_of<TransformComponent>(entity))
    {
        return ancestors;
    }

    entt::entity current = _registry.get<TransformComponent>(entity).parent;
    while (current != entt::null)
    {
        ancestors.push_back(current);

        if (_registry.valid(current) && _registry.all_of<TransformComponent>(current))
        {
            current = _registry.get<TransformComponent>(current).parent;
        }
        else
        {
            break;
        }
    }

    return ancestors;
}

entt::entity SceneGraph::GetRoot(entt::entity entity) const
{
    if (!_registry.valid(entity))
    {
        return entt::null;
    }

    entt::entity current = entity;
    while (_registry.valid(current) && _registry.all_of<TransformComponent>(current))
    {
        entt::entity parent = _registry.get<TransformComponent>(current).parent;
        if (parent == entt::null)
        {
            return current;
        }
        current = parent;
    }

    return current;
}

bool SceneGraph::IsAncestorOf(entt::entity ancestor, entt::entity descendant) const
{
    if (!_registry.valid(ancestor) || !_registry.valid(descendant))
    {
        return false;
    }

    entt::entity current = descendant;
    while (_registry.valid(current) && _registry.all_of<TransformComponent>(current))
    {
        entt::entity parent = _registry.get<TransformComponent>(current).parent;
        if (parent == ancestor)
        {
            return true;
        }
        if (parent == entt::null)
        {
            break;
        }
        current = parent;
    }

    return false;
}

bool SceneGraph::IsDescendantOf(entt::entity descendant, entt::entity ancestor) const
{
    return IsAncestorOf(ancestor, descendant);
}

void SceneGraph::AddRoot(entt::entity entity)
{
    if (std::find(_roots.begin(), _roots.end(), entity) == _roots.end())
    {
        _roots.push_back(entity);
    }
}

void SceneGraph::RemoveRoot(entt::entity entity)
{
    auto it = std::find(_roots.begin(), _roots.end(), entity);
    if (it != _roots.end())
    {
        _roots.erase(it);
    }
}

HS_NS_END
