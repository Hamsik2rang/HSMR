//
//  ProjectLauncherWindow.h
//  Editor
//
//  Created by Claude on 2/10/26.
//

#ifndef __HS_PROJECT_LAUNCHER_WINDOW_H__
#define __HS_PROJECT_LAUNCHER_WINDOW_H__

#include "Precompile.h"

#include "Engine/Window.h"

/*#include "RHI/Swapchain.h"*/ namespace hs { class Swapchain; }
/*#include "Editor/GUI/GUIContext.h"*/ namespace hs { namespace editor { class GUIContext; } }

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ProjectLauncherWindow : public Window
{
public:
    ProjectLauncherWindow(Application* ownerApp);
    ~ProjectLauncherWindow() override;

    // Project selection result
    bool HasSelectedProject() const { return !_selectedProjectPath.empty(); }
    const std::string& GetSelectedProjectPath() const { return _selectedProjectPath; }

    // Check if user wants to create new project
    bool IsNewProjectRequested() const { return _createNewProject; }
    const std::string& GetNewProjectName() const { return _newProjectName; }
    const std::string& GetNewProjectPath() const { return _newProjectPathStr; }

private:
    bool onInitialize() override;
    void onNextFrame() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onPresent() override;
    void onShutdown() override;

    void drawLauncherUI();
    void drawProjectList();
    void drawNewProjectDialog();
    void drawOpenProjectDialog();

    bool openFolderDialog(char* outPath, size_t pathSize);
    bool openFileDialog(char* outPath, size_t pathSize);

    std::string _selectedProjectPath;

    // New project creation
    bool        _createNewProject      = false;
    std::string _newProjectName        = "";
    std::string _newProjectPathStr     = "";

    // UI state
    bool _showNewProjectDialog = false;
    char _newProjectNameBuffer[256] = {0};
    char _newProjectPathBuffer[512] = {0};

    GUIContext* _guiContext = nullptr;
};

HS_NS_EDITOR_END

#endif // __HS_PROJECT_LAUNCHER_WINDOW_H__
