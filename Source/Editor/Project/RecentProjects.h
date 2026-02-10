//
//  RecentProjects.h
//  Editor
//
//  Created by Claude on 2/10/26.
//

#ifndef __HS_RECENT_PROJECTS_H__
#define __HS_RECENT_PROJECTS_H__

#include "Precompile.h"

HS_NS_EDITOR_BEGIN

struct RecentProjectEntry
{
    std::string name       = "";
    std::string path       = "";     // .hsproj file path
    uint64      lastOpened = 0;      // Unix timestamp
    bool        exists     = true;   // File existence check
};

class HS_EDITOR_API RecentProjects
{
public:
    static RecentProjects& Get();

    void Load();
    void Save();

    void AddProject(const std::string& projectFilePath, const std::string& name);
    void RemoveProject(const std::string& projectFilePath);

    const std::vector<RecentProjectEntry>& GetProjects() const { return _projects; }

    void ValidateProjects();    // Check existence of project files

    static constexpr int MAX_RECENT_PROJECTS = 10;

private:
    RecentProjects() = default;
    ~RecentProjects() = default;

    RecentProjects(const RecentProjects&)            = delete;
    RecentProjects& operator=(const RecentProjects&) = delete;

    std::string getConfigPath() const;

    std::vector<RecentProjectEntry> _projects;
};

HS_NS_EDITOR_END

#endif // __HS_RECENT_PROJECTS_H__
