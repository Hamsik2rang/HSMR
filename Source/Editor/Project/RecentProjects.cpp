//
//  RecentProjects.cpp
//  Editor
//
//  Created by Claude on 2/10/26.
//

#include "Editor/Project/RecentProjects.h"

#include "Core/SystemContext.h"
#include "Core/HAL/FileSystem.h"
#include "Core/Log.h"

#include <json.hpp>
#include <fstream>
#include <chrono>
#include <algorithm>

using json = nlohmann::json;

HS_NS_EDITOR_BEGIN

RecentProjects& RecentProjects::Get()
{
    static RecentProjects instance;
    return instance;
}

void RecentProjects::Load()
{
    std::string configPath = getConfigPath();
    if (!hs::FileSystem::Exist(configPath))
    {
        _projects.clear();
        return;
    }

    std::ifstream file(configPath);
    if (!file.is_open())
    {
        HS_LOG(warning, "Failed to open recent projects file: {}", configPath);
        return;
    }

    try
    {
        json j;
        file >> j;
        file.close();

        _projects.clear();

        if (j.contains("projects") && j["projects"].is_array())
        {
            for (const auto& projectJson : j["projects"])
            {
                RecentProjectEntry entry;
                entry.name       = projectJson.value("name", "");
                entry.path       = projectJson.value("path", "");
                entry.lastOpened = projectJson.value("lastOpened", 0ULL);
                entry.exists     = true; // Will be validated later

                if (!entry.path.empty())
                {
                    _projects.push_back(entry);
                }
            }
        }

        // Validate existence
        ValidateProjects();

        HS_LOG(info, "Loaded {} recent projects", _projects.size());
    }
    catch (const std::exception& e)
    {
        HS_LOG(error, "Failed to parse recent projects file: {}", e.what());
        _projects.clear();
    }
}

void RecentProjects::Save()
{
    std::string configPath = getConfigPath();

    // Ensure directory exists
    std::string configDir = hs::FileSystem::GetDirectory(configPath);
    hs::FileSystem::CreateDirectoryRecursive(configDir);

    json j;
    j["projects"] = json::array();

    for (const auto& project : _projects)
    {
        json projectJson;
        projectJson["name"]       = project.name;
        projectJson["path"]       = project.path;
        projectJson["lastOpened"] = project.lastOpened;
        j["projects"].push_back(projectJson);
    }

    std::ofstream file(configPath);
    if (!file.is_open())
    {
        HS_LOG(error, "Failed to save recent projects file: {}", configPath);
        return;
    }

    file << j.dump(4);
    file.close();

    HS_LOG(info, "Saved {} recent projects", _projects.size());
}

void RecentProjects::AddProject(const std::string& projectFilePath, const std::string& name)
{
    if (projectFilePath.empty())
    {
        return;
    }

    // Remove if already exists
    auto it = std::find_if(_projects.begin(), _projects.end(),
        [&projectFilePath](const RecentProjectEntry& entry) {
            return entry.path == projectFilePath;
        });

    if (it != _projects.end())
    {
        _projects.erase(it);
    }

    // Add to front
    RecentProjectEntry entry;
    entry.name   = name;
    entry.path   = projectFilePath;
    entry.exists = true;

    // Get current timestamp
    auto now     = std::chrono::system_clock::now();
    auto epoch   = now.time_since_epoch();
    entry.lastOpened = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();

    _projects.insert(_projects.begin(), entry);

    // Limit size
    if (_projects.size() > MAX_RECENT_PROJECTS)
    {
        _projects.resize(MAX_RECENT_PROJECTS);
    }

    Save();
}

void RecentProjects::RemoveProject(const std::string& projectFilePath)
{
    auto it = std::find_if(_projects.begin(), _projects.end(),
        [&projectFilePath](const RecentProjectEntry& entry) {
            return entry.path == projectFilePath;
        });

    if (it != _projects.end())
    {
        _projects.erase(it);
        Save();
    }
}

void RecentProjects::ValidateProjects()
{
    for (auto& project : _projects)
    {
        project.exists = hs::FileSystem::Exist(project.path);
    }
}

std::string RecentProjects::getConfigPath() const
{
    auto* sysContext = hs::SystemContext::Get();
    if (sysContext && !sysContext->appDataDirectory.empty())
    {
        return sysContext->appDataDirectory + "RecentProjects.json";
    }

    // Fallback to executable directory
    return sysContext->executableDirectory + "RecentProjects.json";
}

HS_NS_EDITOR_END
