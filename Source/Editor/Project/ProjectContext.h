//
//  ProjectContext.h
//  Editor
//
//  Created by Claude on 2/10/26.
//

#ifndef __HS_PROJECT_CONTEXT_H__
#define __HS_PROJECT_CONTEXT_H__

#include "Precompile.h"

HS_NS_EDITOR_BEGIN

struct ProjectSettings
{
    std::string version       = "1.0";
    std::string name          = "";
    std::string engineVersion = "0.1.0";
    std::string defaultScene  = "";
    std::string buildTarget   = "Windows";
    std::string renderAPI     = "Vulkan";
};

class HS_EDITOR_API ProjectContext
{
public:
    static ProjectContext& Get();

    // Project management
    bool CreateProject(const std::string& folderPath, const std::string& name);
    bool OpenProject(const std::string& projectFilePath);
    void CloseProject();

    bool IsProjectOpen() const { return _isOpen; }

    // Path accessors
    const std::string& GetProjectPath() const { return _projectPath; }
    const std::string& GetProjectFilePath() const { return _projectFilePath; }
    const std::string& GetProjectName() const { return _settings.name; }

    std::string GetAssetPath() const;
    std::string GetScenePath() const;
    std::string GetSettingsPath() const;
    std::string GetResolvedScenePath(const std::string& scenePath) const;
    std::string GetResolvedDefaultScenePath() const;
    std::string MakeProjectRelativePath(const std::string& path) const;

    // Settings
    const ProjectSettings& GetSettings() const { return _settings; }
    void SetDefaultScene(const std::string& scenePath);
    bool SaveSettings();

private:
    ProjectContext() = default;
    ~ProjectContext() = default;

    ProjectContext(const ProjectContext&)            = delete;
    ProjectContext& operator=(const ProjectContext&) = delete;

    bool loadProjectFile(const std::string& path);
    bool saveProjectFile();
    void createDefaultDirectories();

    bool        _isOpen          = false;
    std::string _projectPath     = "";   // Project root folder
    std::string _projectFilePath = "";   // .hsproj file path
    ProjectSettings _settings;
};

HS_NS_EDITOR_END

#endif // __HS_PROJECT_CONTEXT_H__
