//
//  Scene.cpp
//  HSMR
//
//  Created for lightweight prototyping framework
//
#include "Scene.h"
#include "Core/Log.h"

#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

HS_NS_BEGIN

Scene::~Scene()
{
    Clear();
}

bool Scene::LoadFromJSON(const std::string& path)
{
    Clear();

    // Read JSON file
    std::ifstream file(path);
    if (!file.is_open())
    {
        HS_LOG(error,"Failed to open scene file: %s", path.c_str());
        return false;
    }

    // Extract base path for relative resource paths
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        _basePath = path.substr(0, lastSlash + 1);
    }

    json sceneJson;
    try
    {
        file >> sceneJson;
    }
    catch (const json::parse_error& e)
    {
        HS_LOG(error,"JSON parse error: %s", e.what());
        return false;
    }
    file.close();

    // Parse scene name
    if (sceneJson.contains("name"))
    {
        _name = sceneJson["name"].get<std::string>();
    }

    // Parse camera configuration
    if (sceneJson.contains("camera"))
    {
        if (!parseCamera(&sceneJson["camera"]))
        {
            HS_LOG(warning,"Failed to parse camera configuration");
        }
    }

    // Parse shaders
    if (sceneJson.contains("shaders"))
    {
        if (!parseShaders(&sceneJson["shaders"]))
        {
            HS_LOG(warning,"Failed to parse shaders");
        }
    }

    // Parse textures
    if (sceneJson.contains("textures"))
    {
        if (!parseTextures(&sceneJson["textures"]))
        {
            HS_LOG(warning,"Failed to parse textures");
        }
    }

    // Parse models
    if (sceneJson.contains("models"))
    {
        if (!parseModels(&sceneJson["models"]))
        {
            HS_LOG(warning,"Failed to parse models");
        }
    }

    // Parse objects
    if (sceneJson.contains("objects"))
    {
        if (!parseObjects(&sceneJson["objects"]))
        {
            HS_LOG(warning,"Failed to parse objects");
        }
    }

    HS_LOG(info,"Loaded scene: %s with %zu objects", _name.c_str(), _objects.size());
    return true;
}

void Scene::Clear()
{
    _name.clear();
    _basePath.clear();

    _models.clear();
    _shaders.clear();
    _textures.clear();
    _objects.clear();

    _modelNameMap.clear();
    _shaderNameMap.clear();
    _textureNameMap.clear();

    _selectedObject = nullptr;
}

void Scene::ApplyCameraConfig(Camera* camera) const
{
    if (!camera) return;

    camera->SetPosition(_cameraConfig.position);
    camera->SetTarget(_cameraConfig.target);
    camera->SetPerspective(
        glm::radians(_cameraConfig.fov),
        camera->GetAspectRatio(),
        _cameraConfig.nearZ,
        _cameraConfig.farZ
    );
    camera->Update();
}

SceneObject* Scene::GetObject(size_t index)
{
    if (index >= _objects.size()) return nullptr;
    return &_objects[index];
}

SceneObject* Scene::FindObjectByName(const std::string& name)
{
    for (auto& obj : _objects)
    {
        if (obj.GetName() == name)
        {
            return &obj;
        }
    }
    return nullptr;
}

int32 Scene::FindModelIndex(const std::string& name) const
{
    auto it = _modelNameMap.find(name);
    if (it != _modelNameMap.end())
    {
        return it->second;
    }
    return -1;
}

int32 Scene::FindShaderIndex(const std::string& name) const
{
    auto it = _shaderNameMap.find(name);
    if (it != _shaderNameMap.end())
    {
        return it->second;
    }
    return -1;
}

int32 Scene::FindTextureIndex(const std::string& name) const
{
    auto it = _textureNameMap.find(name);
    if (it != _textureNameMap.end())
    {
        return it->second;
    }
    return -1;
}

bool Scene::parseCamera(const void* jsonCamera)
{
    const json& cam = *static_cast<const json*>(jsonCamera);

    if (cam.contains("position") && cam["position"].is_array())
    {
        auto& pos = cam["position"];
        _cameraConfig.position = glm::vec3(
            pos[0].get<float>(),
            pos[1].get<float>(),
            pos[2].get<float>()
        );
    }

    if (cam.contains("target") && cam["target"].is_array())
    {
        auto& target = cam["target"];
        _cameraConfig.target = glm::vec3(
            target[0].get<float>(),
            target[1].get<float>(),
            target[2].get<float>()
        );
    }

    if (cam.contains("fov"))
    {
        _cameraConfig.fov = cam["fov"].get<float>();
    }

    if (cam.contains("near"))
    {
        _cameraConfig.nearZ = cam["near"].get<float>();
    }

    if (cam.contains("far"))
    {
        _cameraConfig.farZ = cam["far"].get<float>();
    }

    return true;
}

bool Scene::parseShaders(const void* jsonShaders)
{
    const json& shaders = *static_cast<const json*>(jsonShaders);

    if (!shaders.is_array()) return false;

    for (const auto& shader : shaders)
    {
        ShaderData data;

        if (shader.contains("name"))
        {
            data.name = shader["name"].get<std::string>();
        }

        if (shader.contains("path"))
        {
            data.path = _basePath + shader["path"].get<std::string>();
        }

        if (shader.contains("stages") && shader["stages"].is_array())
        {
            for (const auto& stage : shader["stages"])
            {
                data.stages.push_back(stage.get<std::string>());
            }
        }

        // Invoke shader load callback if set
        if (_shaderLoadCallback)
        {
            _shaderLoadCallback(data);
        }

        _shaderNameMap[data.name] = static_cast<int32>(_shaders.size());
        _shaders.push_back(std::move(data));
    }

    return true;
}

bool Scene::parseTextures(const void* jsonTextures)
{
    const json& textures = *static_cast<const json*>(jsonTextures);

    if (!textures.is_array()) return false;

    for (const auto& texture : textures)
    {
        TextureData data;

        if (texture.contains("name"))
        {
            data.name = texture["name"].get<std::string>();
        }

        if (texture.contains("path"))
        {
            data.path = _basePath + texture["path"].get<std::string>();
        }

        // Invoke texture load callback if set
        if (_textureLoadCallback)
        {
            data.texture = _textureLoadCallback(data);
        }

        _textureNameMap[data.name] = static_cast<int32>(_textures.size());
        _textures.push_back(std::move(data));
    }

    return true;
}

bool Scene::parseModels(const void* jsonModels)
{
    const json& models = *static_cast<const json*>(jsonModels);

    if (!models.is_array()) return false;

    for (const auto& model : models)
    {
        ModelData data;

        if (model.contains("name"))
        {
            data.name = model["name"].get<std::string>();
        }

        if (model.contains("path"))
        {
            data.path = _basePath + model["path"].get<std::string>();
        }

        if (model.contains("settings"))
        {
            const auto& settings = model["settings"];

            if (settings.contains("generateNormals"))
            {
                data.generateNormals = settings["generateNormals"].get<bool>();
            }

            if (settings.contains("generateTangents"))
            {
                data.generateTangents = settings["generateTangents"].get<bool>();
            }

            if (settings.contains("scale"))
            {
                data.scale = settings["scale"].get<float>();
            }
        }

        // Invoke model load callback if set
        if (_modelLoadCallback)
        {
            data.modelResource = _modelLoadCallback(data);
        }

        _modelNameMap[data.name] = static_cast<int32>(_models.size());
        _models.push_back(std::move(data));
    }

    return true;
}

bool Scene::parseObjects(const void* jsonObjects)
{
    const json& objects = *static_cast<const json*>(jsonObjects);

    if (!objects.is_array()) return false;

    for (const auto& obj : objects)
    {
        SceneObject sceneObj;

        if (obj.contains("name"))
        {
            sceneObj.SetName(obj["name"].get<std::string>());
        }

        if (obj.contains("model"))
        {
            std::string modelName = obj["model"].get<std::string>();
            int32 modelIdx = FindModelIndex(modelName);
            sceneObj.SetModelIndex(modelIdx);

            // Set bounds from model if available
            if (modelIdx >= 0 && modelIdx < static_cast<int32>(_models.size()))
            {
                sceneObj.SetLocalBounds(_models[modelIdx].bounds);
            }
        }

        if (obj.contains("shader"))
        {
            std::string shaderName = obj["shader"].get<std::string>();
            sceneObj.SetShaderIndex(FindShaderIndex(shaderName));
        }

        if (obj.contains("transform"))
        {
            const auto& transform = obj["transform"];

            if (transform.contains("position") && transform["position"].is_array())
            {
                const auto& pos = transform["position"];
                sceneObj.SetPosition(glm::vec3(
                    pos[0].get<float>(),
                    pos[1].get<float>(),
                    pos[2].get<float>()
                ));
            }

            if (transform.contains("rotation") && transform["rotation"].is_array())
            {
                const auto& rot = transform["rotation"];
                sceneObj.SetRotation(glm::vec3(
                    glm::radians(rot[0].get<float>()),
                    glm::radians(rot[1].get<float>()),
                    glm::radians(rot[2].get<float>())
                ));
            }

            if (transform.contains("scale") && transform["scale"].is_array())
            {
                const auto& scale = transform["scale"];
                sceneObj.SetScale(glm::vec3(
                    scale[0].get<float>(),
                    scale[1].get<float>(),
                    scale[2].get<float>()
                ));
            }
        }

        // Material properties
        if (obj.contains("material"))
        {
            const auto& material = obj["material"];

            if (material.contains("baseColor") && material["baseColor"].is_array())
            {
                const auto& color = material["baseColor"];
                sceneObj.SetBaseColor(glm::vec4(
                    color[0].get<float>(),
                    color[1].get<float>(),
                    color[2].get<float>(),
                    color.size() > 3 ? color[3].get<float>() : 1.0f
                ));
            }

            if (material.contains("metallic"))
            {
                sceneObj.SetMetallic(material["metallic"].get<float>());
            }

            if (material.contains("roughness"))
            {
                sceneObj.SetRoughness(material["roughness"].get<float>());
            }
        }

        if (obj.contains("visible"))
        {
            sceneObj.SetVisible(obj["visible"].get<bool>());
        }

        _objects.push_back(std::move(sceneObj));
    }

    return true;
}

HS_NS_END
