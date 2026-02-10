//
//  ProjectContext.cpp
//  Editor
//
//  Created by Claude on 2/10/26.
//

#include "Editor/Project/ProjectContext.h"

#include "Core/HAL/FileSystem.h"
#include "Core/Log.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

HS_EDITOR_NS_BEGIN

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
        HS_LOG(error, "Failed to create project directory: {}", projectPath);
        return false;
    }

    // Set paths
    _projectPath     = projectPath;
    _projectFilePath = projectPath + name + ".hsproj";

    // Set settings
    _settings.name          = name;
    _settings.version       = "1.0";
    _settings.engineVersion = "0.1.0";
    _settings.defaultScene  = "";
    _settings.buildTarget   = "Windows";
    _settings.renderAPI     = "Vulkan";

    // Create default directories
    createDefaultDirectories();

    // Save project file
    if (!saveProjectFile())
    {
        HS_LOG(error, "Failed to save project file");
        return false;
    }

    _isOpen = true;

    HS_LOG(info, "Project created: {} at {}", name, projectPath);
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
        HS_LOG(error, "Project file not found: {}", projectFilePath);
        return false;
    }

    if (!loadProjectFile(projectFilePath))
    {
        HS_LOG(error, "Failed to load project file: {}", projectFilePath);
        return false;
    }

    _projectFilePath = projectFilePath;
    _projectPath     = hs::FileSystem::GetDirectory(projectFilePath);
    _isOpen          = true;

    HS_LOG(info, "Project opened: {} at {}", _settings.name, _projectPath);
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
    return _projectPath + "Scenes" + HS_DIR_SEPERATOR;
}

std::string ProjectContext::GetSettingsPath() const
{
    if (_projectPath.empty())
    {
        return "";
    }
    return _projectPath + "ProjectSettings" + HS_DIR_SEPERATOR;
}

void ProjectContext::SetDefaultScene(const std::string& scenePath)
{
    _settings.defaultScene = scenePath;
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
    j["directories"]["scenes"] = "Scenes";
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

    // Create Scenes folder
    hs::FileSystem::CreateDirectoryRecursive(GetScenePath());

    // Create Scripts folder (for future)
    hs::FileSystem::CreateDirectoryRecursive(_projectPath + "Scripts" + HS_DIR_SEPERATOR);

    // Create ProjectSettings folder
    hs::FileSystem::CreateDirectoryRecursive(GetSettingsPath());
}

HS_EDITOR_NS_END
