//
//  SceneSerializer.cpp
//  Engine
//
//  Scene serialization implementation
//

#include "Scene/SceneSerializer.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

#include "Core/Log.h"
#include "Core/HAL/FileSystem.h"

#include <json.hpp>
#include <fstream>
#include <unordered_map>

using json = nlohmann::json;

HS_NS_BEGIN

// Helper functions for GLM type serialization
namespace
{
    json serializeVec3(const glm::vec3& v)
    {
        return json::array({v.x, v.y, v.z});
    }

    json serializeVec4(const glm::vec4& v)
    {
        return json::array({v.x, v.y, v.z, v.w});
    }

    json serializeQuat(const glm::quat& q)
    {
        return json::array({q.x, q.y, q.z, q.w});
    }

    glm::vec3 deserializeVec3(const json& j)
    {
        if (j.is_array() && j.size() >= 3)
        {
            return glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
        }
        return glm::vec3(0.0f);
    }

    glm::vec4 deserializeVec4(const json& j)
    {
        if (j.is_array() && j.size() >= 4)
        {
            return glm::vec4(j[0].get<float>(), j[1].get<float>(),
                             j[2].get<float>(), j[3].get<float>());
        }
        return glm::vec4(0.0f);
    }

    glm::quat deserializeQuat(const json& j)
    {
        if (j.is_array() && j.size() >= 4)
        {
            // Order: x, y, z, w
            return glm::quat(j[3].get<float>(), j[0].get<float>(),
                             j[1].get<float>(), j[2].get<float>());
        }
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    // Serialize individual components
    json serializeTagComponent(const TagComponent& tag)
    {
        json j;
        j["name"] = tag.name;
        j["layer"] = tag.layer;
        j["isStatic"] = tag.isStatic;
        j["isActive"] = tag.isActive;
        return j;
    }

    json serializeTransformComponent(const TransformComponent& transform, Scene* scene)
    {
        json j;
        j["position"] = serializeVec3(transform.position);
        j["rotation"] = serializeQuat(transform.rotation);
        j["scale"] = serializeVec3(transform.scale);

        // Parent reference (we'll use entity name for now, could use UUID later)
        if (transform.parent != entt::null)
        {
            Entity parentEntity = scene->GetEntity(transform.parent);
            if (parentEntity.IsValid() && parentEntity.HasComponent<TagComponent>())
            {
                j["parent"] = parentEntity.GetComponent<TagComponent>().name;
            }
        }

        return j;
    }

    json serializeMeshRendererComponent(const MeshRendererComponent& mr)
    {
        json j;
        // We store asset paths, but currently MeshRendererComponent doesn't track them
        // This is a limitation - for now we just save the state
        j["castShadow"] = mr.castShadow;
        j["receiveShadow"] = mr.receiveShadow;
        j["isVisible"] = mr.isVisible;
        j["renderLayerMask"] = mr.renderLayerMask;

        // TODO: Store mesh/material asset paths when asset system is integrated
        // j["mesh"] = meshAssetPath;
        // j["materials"] = materialPaths;

        return j;
    }

    json serializeCameraComponent(const CameraComponent& cam)
    {
        json j;
        j["projectionType"] = (cam.projectionType == CameraComponent::EProjectionType::Perspective)
            ? "Perspective" : "Orthographic";
        j["fov"] = cam.fov;
        j["orthoSize"] = cam.orthoSize;
        j["nearPlane"] = cam.nearPlane;
        j["farPlane"] = cam.farPlane;
        j["isActive"] = cam.isActive;
        j["isPrimary"] = cam.isPrimary;
        j["viewport"] = serializeVec4(cam.viewport);
        return j;
    }

    json serializeLightComponent(const LightComponent& light)
    {
        json j;

        const char* typeStr = "Directional";
        switch (light.type)
        {
            case ELightType::Directional: typeStr = "Directional"; break;
            case ELightType::Point:       typeStr = "Point"; break;
            case ELightType::Spot:        typeStr = "Spot"; break;
        }
        j["type"] = typeStr;

        j["color"] = serializeVec3(light.color);
        j["intensity"] = light.intensity;
        j["range"] = light.range;
        j["attenuation"] = light.attenuation;
        j["innerConeAngle"] = light.innerConeAngle;
        j["outerConeAngle"] = light.outerConeAngle;
        j["castShadow"] = light.castShadow;
        j["shadowBias"] = light.shadowBias;
        j["shadowMapResolution"] = light.shadowMapResolution;
        j["isEnabled"] = light.isEnabled;

        return j;
    }

    // Deserialize individual components
    void deserializeTagComponent(Entity entity, const json& j)
    {
        auto& tag = entity.GetComponent<TagComponent>();

        if (j.contains("name")) tag.name = j["name"].get<std::string>();
        if (j.contains("layer")) tag.layer = j["layer"].get<uint32>();
        if (j.contains("isStatic")) tag.isStatic = j["isStatic"].get<bool>();
        if (j.contains("isActive")) tag.isActive = j["isActive"].get<bool>();
    }

    void deserializeTransformComponent(Entity entity, const json& j)
    {
        auto& transform = entity.GetComponent<TransformComponent>();

        if (j.contains("position")) transform.position = deserializeVec3(j["position"]);
        if (j.contains("rotation")) transform.rotation = deserializeQuat(j["rotation"]);
        if (j.contains("scale")) transform.scale = deserializeVec3(j["scale"]);

        transform.isDirty = true;
    }

    void deserializeMeshRendererComponent(Entity entity, const json& j)
    {
        if (!entity.HasComponent<MeshRendererComponent>())
        {
            entity.AddComponent<MeshRendererComponent>();
        }

        auto& mr = entity.GetComponent<MeshRendererComponent>();

        if (j.contains("castShadow")) mr.castShadow = j["castShadow"].get<bool>();
        if (j.contains("receiveShadow")) mr.receiveShadow = j["receiveShadow"].get<bool>();
        if (j.contains("isVisible")) mr.isVisible = j["isVisible"].get<bool>();
        if (j.contains("renderLayerMask")) mr.renderLayerMask = j["renderLayerMask"].get<uint32>();

        // TODO: Load mesh/material from asset paths
    }

    void deserializeCameraComponent(Entity entity, const json& j)
    {
        if (!entity.HasComponent<CameraComponent>())
        {
            entity.AddComponent<CameraComponent>();
        }

        auto& cam = entity.GetComponent<CameraComponent>();

        if (j.contains("projectionType"))
        {
            std::string typeStr = j["projectionType"].get<std::string>();
            cam.projectionType = (typeStr == "Orthographic")
                ? CameraComponent::EProjectionType::Orthographic
                : CameraComponent::EProjectionType::Perspective;
        }

        if (j.contains("fov")) cam.fov = j["fov"].get<float>();
        if (j.contains("orthoSize")) cam.orthoSize = j["orthoSize"].get<float>();
        if (j.contains("nearPlane")) cam.nearPlane = j["nearPlane"].get<float>();
        if (j.contains("farPlane")) cam.farPlane = j["farPlane"].get<float>();
        if (j.contains("isActive")) cam.isActive = j["isActive"].get<bool>();
        if (j.contains("isPrimary")) cam.isPrimary = j["isPrimary"].get<bool>();
        if (j.contains("viewport")) cam.viewport = deserializeVec4(j["viewport"]);
    }

    void deserializeLightComponent(Entity entity, const json& j)
    {
        ELightType lightType = ELightType::Directional;

        if (j.contains("type"))
        {
            std::string typeStr = j["type"].get<std::string>();
            if (typeStr == "Point") lightType = ELightType::Point;
            else if (typeStr == "Spot") lightType = ELightType::Spot;
        }

        if (!entity.HasComponent<LightComponent>())
        {
            entity.AddComponent<LightComponent>(lightType);
        }

        auto& light = entity.GetComponent<LightComponent>();
        light.type = lightType;

        if (j.contains("color")) light.color = deserializeVec3(j["color"]);
        if (j.contains("intensity")) light.intensity = j["intensity"].get<float>();
        if (j.contains("range")) light.range = j["range"].get<float>();
        if (j.contains("attenuation")) light.attenuation = j["attenuation"].get<float>();
        if (j.contains("innerConeAngle")) light.innerConeAngle = j["innerConeAngle"].get<float>();
        if (j.contains("outerConeAngle")) light.outerConeAngle = j["outerConeAngle"].get<float>();
        if (j.contains("castShadow")) light.castShadow = j["castShadow"].get<bool>();
        if (j.contains("shadowBias")) light.shadowBias = j["shadowBias"].get<float>();
        if (j.contains("shadowMapResolution")) light.shadowMapResolution = j["shadowMapResolution"].get<uint32>();
        if (j.contains("isEnabled")) light.isEnabled = j["isEnabled"].get<bool>();
    }
}

SceneSerializer::SceneSerializer(Scene* scene)
    : _scene(scene)
{
}

bool SceneSerializer::SaveToFile(const std::string& filePath)
{
    std::string jsonStr = SaveToString();
    if (jsonStr.empty())
    {
        return false;
    }

    std::ofstream file(filePath);
    if (!file.is_open())
    {
        HS_LOG(error, "[SceneSerializer] Failed to open file for writing: {}", filePath);
        return false;
    }

    file << jsonStr;
    file.close();

    HS_LOG(info, "[SceneSerializer] Scene saved to: {}", filePath);
    return true;
}

std::string SceneSerializer::SaveToString()
{
    if (!_scene)
    {
        HS_LOG(error, "[SceneSerializer] No scene to serialize");
        return "";
    }

    json root;
    root["version"] = SCHEMA_VERSION;
    root["name"] = _scene->GetName();

    json entitiesArray = json::array();

    // First pass: collect all root entities (no parent)
    std::vector<Entity> rootEntities;
    auto view = _scene->View<TagComponent, TransformComponent>();

    for (auto entityHandle : view)
    {
        Entity entity = _scene->GetEntity(entityHandle);
        if (!entity.IsValid()) continue;

        const auto& transform = entity.GetComponent<TransformComponent>();
        if (!transform.HasParent())
        {
            rootEntities.push_back(entity);
        }
    }

    // Recursive function to serialize entity and children
    std::function<json(Entity)> serializeEntity = [&](Entity entity) -> json
    {
        json entityJson;

        // Tag component (always present)
        if (entity.HasComponent<TagComponent>())
        {
            entityJson["tag"] = serializeTagComponent(entity.GetComponent<TagComponent>());
        }

        // Transform component
        if (entity.HasComponent<TransformComponent>())
        {
            entityJson["transform"] = serializeTransformComponent(
                entity.GetComponent<TransformComponent>(), _scene);
        }

        // Optional components
        if (entity.HasComponent<MeshRendererComponent>())
        {
            entityJson["meshRenderer"] = serializeMeshRendererComponent(
                entity.GetComponent<MeshRendererComponent>());
        }

        if (entity.HasComponent<CameraComponent>())
        {
            entityJson["camera"] = serializeCameraComponent(
                entity.GetComponent<CameraComponent>());
        }

        if (entity.HasComponent<LightComponent>())
        {
            entityJson["light"] = serializeLightComponent(
                entity.GetComponent<LightComponent>());
        }

        // Children
        const auto& transform = entity.GetComponent<TransformComponent>();
        if (!transform.children.empty())
        {
            json childrenArray = json::array();
            for (auto childHandle : transform.children)
            {
                Entity childEntity = _scene->GetEntity(childHandle);
                if (childEntity.IsValid())
                {
                    childrenArray.push_back(serializeEntity(childEntity));
                }
            }
            entityJson["children"] = childrenArray;
        }

        return entityJson;
    };

    // Serialize all root entities (children are nested inside)
    for (auto& entity : rootEntities)
    {
        entitiesArray.push_back(serializeEntity(entity));
    }

    root["entities"] = entitiesArray;

    return root.dump(2);  // Pretty print with 2-space indent
}

bool SceneSerializer::LoadFromFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        HS_LOG(error, "[SceneSerializer] Failed to open file: {}", filePath);
        return false;
    }

    std::string jsonStr((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    return LoadFromString(jsonStr);
}

bool SceneSerializer::LoadFromString(const std::string& jsonString)
{
    if (!_scene)
    {
        HS_LOG(error, "[SceneSerializer] No scene to deserialize into");
        return false;
    }

    try
    {
        json root = json::parse(jsonString);

        // Check version
        if (root.contains("version"))
        {
            std::string version = root["version"].get<std::string>();
            HS_LOG(info, "[SceneSerializer] Loading scene version: {}", version);
        }

        // Scene name
        if (root.contains("name"))
        {
            _scene->SetName(root["name"].get<std::string>());
        }

        // Clear existing entities
        ClearScene();

        // Recursive function to deserialize entity and children
        std::function<Entity(const json&, Entity)> deserializeEntity =
            [&](const json& entityJson, Entity parent) -> Entity
        {
            // Get entity name from tag
            std::string entityName = "Entity";
            if (entityJson.contains("tag") && entityJson["tag"].contains("name"))
            {
                entityName = entityJson["tag"]["name"].get<std::string>();
            }

            // Create entity
            Entity entity = parent.IsValid()
                ? _scene->CreateChildEntity(parent, entityName)
                : _scene->CreateEntity(entityName);

            // Deserialize components
            if (entityJson.contains("tag"))
            {
                deserializeTagComponent(entity, entityJson["tag"]);
            }

            if (entityJson.contains("transform"))
            {
                deserializeTransformComponent(entity, entityJson["transform"]);
            }

            if (entityJson.contains("meshRenderer"))
            {
                deserializeMeshRendererComponent(entity, entityJson["meshRenderer"]);
            }

            if (entityJson.contains("camera"))
            {
                deserializeCameraComponent(entity, entityJson["camera"]);
            }

            if (entityJson.contains("light"))
            {
                deserializeLightComponent(entity, entityJson["light"]);
            }

            // Deserialize children recursively
            if (entityJson.contains("children") && entityJson["children"].is_array())
            {
                for (const auto& childJson : entityJson["children"])
                {
                    deserializeEntity(childJson, entity);
                }
            }

            return entity;
        };

        // Deserialize all root entities
        if (root.contains("entities") && root["entities"].is_array())
        {
            for (const auto& entityJson : root["entities"])
            {
                deserializeEntity(entityJson, Entity{});
            }
        }

        // Update transforms
        _scene->Update(0.0f);

        HS_LOG(info, "[SceneSerializer] Scene loaded successfully");
        return true;
    }
    catch (const json::exception& e)
    {
        HS_LOG(error, "[SceneSerializer] JSON parse error: {}", e.what());
        return false;
    }
}

void SceneSerializer::ClearScene()
{
    if (!_scene) return;

    // Collect all entities first (to avoid iterator invalidation)
    std::vector<Entity> entities;
    auto view = _scene->View<TagComponent>();
    for (auto entityHandle : view)
    {
        entities.push_back(_scene->GetEntity(entityHandle));
    }

    // Destroy all entities
    for (auto& entity : entities)
    {
        if (entity.IsValid())
        {
            _scene->DestroyEntity(entity);
        }
    }
}

HS_NS_END
