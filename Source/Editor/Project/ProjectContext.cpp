//
//  ProjectContext.cpp
//  Editor
//
//  Created by Claude on 2/10/26.
//

#include "Editor/Project/ProjectContext.h"

#include "Core/HAL/FileSystem.h"
#include "Core/Log.h"
#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"
#include "Scene/Components/Components.h"

#include <json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

HS_NS_EDITOR_BEGIN

namespace
{
constexpr const char* s_defaultSceneRelativePath = "Assets/Scenes/Main.scene";

void populateStarterScene(Scene& scene, const std::string& sceneName)
{
    scene.SetName(sceneName);

    Entity cameraEntity = scene.CreateEntity("Main Camera");
    auto& camera = cameraEntity.AddComponent<CameraComponent>();
    camera.isPrimary = true;
    camera.isActive = true;
    camera.priority = 100;

    Entity lightEntity = scene.CreateEntity("Directional Light");
    auto& light = lightEntity.AddComponent<LightComponent>();
    light.type = ELightType::Directional;
    auto& lightTransform = lightEntity.GetComponent<TransformComponent>();
    lightTransform.SetPosition(glm::vec3(0.0f, 3.0f, 0.0f));
    lightTransform.SetEulerAngles(glm::vec3(45.0f, -45.0f, 0.0f));

    scene.Update(0.0f);
}
}

ProjectContext& ProjectContext::Get()
{
    static ProjectContext instance;
    return instance;
}

bool ProjectContext::CreateProject(const std::string& folderPath, const std::string& name)
{
    if (folderPath.empty() || name.empty())
    {
        HS_LOG(error, "Invalid project path or name");
        return false;
    }

    // Create project folder
    std::string projectPath = folderPath;
    if (!projectPath.empty() && projectPath.back() != HS_DIR_SEPERATOR)
    {
        projectPath += HS_DIR_SEPERATOR;
    }

    if (!hs::FileSystem::CreateDirectoryRecursive(projectPath))
    {
        HS_LOG(error, "Failed to create project directory: {}", projectPath.c_str());
        return false;
    }

    // Set paths
    _projectPath     = projectPath;
    _projectFilePath = projectPath + name + ".hsproj";

    // Set settings
    _settings.name          = name;
    _settings.version       = "1.0";
    _settings.engineVersion = "0.1.0";
    _settings.defaultScene  = s_defaultSceneRelativePath;
    _settings.buildTarget   = "Windows";
    _settings.renderAPI     = "Vulkan";

    // Create default directories
    createDefaultDirectories();

    Scene defaultScene("Main");
    populateStarterScene(defaultScene, "Main");

    SceneSerializer serializer(&defaultScene);
    const std::string defaultScenePath = GetResolvedDefaultScenePath();
    if (!serializer.SaveToFile(defaultScenePath))
    {
        HS_LOG(error, "Failed to save default scene: {}", defaultScenePath.c_str());
        return false;
    }

    // Save project file
    if (!saveProjectFile())
    {
        HS_LOG(error, "Failed to save project file");
        return false;
    }

    _isOpen = true;

    HS_LOG(info, "Project created: {} at {}", name.c_str(), projectPath.c_str());
    return true;
}

bool ProjectContext::OpenProject(const std::string& projectFilePath)
{
    if (projectFilePath.empty())
    {
        HS_LOG(error, "Invalid project file path");
        return false;
    }

    if (!hs::FileSystem::Exist(projectFilePath))
    {
        HS_LOG(error, "Project file not found: {}", projectFilePath.c_str());
        return false;
    }

    if (!loadProjectFile(projectFilePath))
    {
        HS_LOG(error, "Failed to load project file: {}", projectFilePath.c_str());
        return false;
    }

    _projectFilePath = projectFilePath;
    _projectPath     = hs::FileSystem::GetDirectory(projectFilePath);
    _isOpen          = true;

    HS_LOG(info, "Project opened: {} at {}", _settings.name.c_str(), _projectPath.c_str());
    return true;
}

void ProjectContext::CloseProject()
{
    if (_isOpen)
    {
        SaveSettings();
        _isOpen          = false;
        _projectPath     = "";
        _projectFilePath = "";
        _settings        = ProjectSettings();

        HS_LOG(info, "Project closed");
    }
}

std::string ProjectContext::GetAssetPath() const
{
    if (_projectPath.empty())
    {
        return "";
    }
    return _projectPath + "Assets" + HS_DIR_SEPERATOR;
}

std::string ProjectContext::GetScenePath() const
{
    if (_projectPath.empty())
    {
        return "";
    }
    return GetAssetPath() + "Scenes" + HS_DIR_SEPERATOR;
}

std::string ProjectContext::GetSettingsPath() const
{
    if (_projectPath.empty())
    {
        return "";
    }
    return _projectPath + "ProjectSettings" + HS_DIR_SEPERATOR;
}

std::string ProjectContext::GetResolvedScenePath(const std::string& scenePath) const
{
    if (scenePath.empty() || _projectPath.empty())
    {
        return "";
    }

    std::filesystem::path path(scenePath);
    if (path.is_absolute())
    {
        return path.lexically_normal().string();
    }

    return (std::filesystem::path(_projectPath) / path).lexically_normal().string();
}

std::string ProjectContext::GetResolvedDefaultScenePath() const
{
    return GetResolvedScenePath(_settings.defaultScene);
}

std::string ProjectContext::MakeProjectRelativePath(const std::string& path) const
{
    if (path.empty() || _projectPath.empty())
    {
        return path;
    }

    std::filesystem::path absolutePath = std::filesystem::path(path).lexically_normal();
    std::filesystem::path projectRoot = std::filesystem::path(_projectPath).lexically_normal();

    if (!absolutePath.is_absolute())
    {
        absolutePath = (projectRoot / absolutePath).lexically_normal();
    }

    std::error_code errorCode;
    std::filesystem::path relativePath = std::filesystem::relative(absolutePath, projectRoot, errorCode);
    if (!errorCode && !relativePath.empty() && relativePath.generic_string().find("..") != 0)
    {
        return relativePath.generic_string();
    }

    return absolutePath.string();
}

void ProjectContext::SetDefaultScene(const std::string& scenePath)
{
    _settings.defaultScene = MakeProjectRelativePath(scenePath);
    SaveSettings();
}

bool ProjectContext::SaveSettings()
{
    return saveProjectFile();
}

bool ProjectContext::loadProjectFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    try
    {
        json j;
        file >> j;
        file.close();

        _settings.version       = j.value("version", "1.0");
        _settings.name          = j.value("name", "Untitled");
        _settings.engineVersion = j.value("engineVersion", "0.1.0");

        if (j.contains("settings"))
        {
            auto& settings          = j["settings"];
            _settings.defaultScene  = settings.value("defaultScene", "");
            _settings.buildTarget   = settings.value("buildTarget", "Windows");
            _settings.renderAPI     = settings.value("renderAPI", "Vulkan");
        }

        return true;
    }
    catch (const std::exception& e)
    {
        HS_LOG(error, "Failed to parse project file: {}", e.what());
        return false;
    }
}

bool ProjectContext::saveProjectFile()
{
    if (_projectFilePath.empty())
    {
        return false;
    }

    json j;
    j["version"]       = _settings.version;
    j["name"]          = _settings.name;
    j["engineVersion"] = _settings.engineVersion;

    j["settings"]["defaultScene"] = _settings.defaultScene;
    j["settings"]["buildTarget"]  = _settings.buildTarget;
    j["settings"]["renderAPI"]    = _settings.renderAPI;

    j["directories"]["assets"] = "Assets";
    j["directories"]["scenes"] = "Assets/Scenes";
    j["directories"]["scripts"] = "Scripts";

    std::ofstream file(_projectFilePath);
    if (!file.is_open())
    {
        return false;
    }

    file << j.dump(4);
    file.close();

    return true;
}

void ProjectContext::createDefaultDirectories()
{
    // Create Assets folder
    hs::FileSystem::CreateDirectoryRecursive(GetAssetPath());
    hs::FileSystem::CreateDirectoryRecursive(GetAssetPath() + "Models");
    hs::FileSystem::CreateDirectoryRecursive(GetAssetPath() + "Textures");
    hs::FileSystem::CreateDirectoryRecursive(GetAssetPath() + "Materials");
    hs::FileSystem::CreateDirectoryRecursive(GetScenePath());

    // Create Scripts folder (for future)
    hs::FileSystem::CreateDirectoryRecursive(_projectPath + "Scripts" + HS_DIR_SEPERATOR);

    // Create ProjectSettings folder
    hs::FileSystem::CreateDirectoryRecursive(GetSettingsPath());
}

HS_NS_EDITOR_END
