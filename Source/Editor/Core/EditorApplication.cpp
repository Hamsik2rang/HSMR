#include "Editor/Core/EditorApplication.h"

#include "Core/Log.h"
#include "Core/HAL/Timer.h"
#include "Core/Native/NativeWindow.h"

#include "Resource/ObjectManager.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorWindow.h"
#include "Editor/Project/ProjectLauncherWindow.h"
#include "Editor/Project/ProjectContext.h"
#include "Editor/Project/RecentProjects.h"
#include "Editor/Asset/AssetDatabase.h"

#include "Core/HAL/CommandLine.h"

#if defined(__APPLE__)
#include "Platform/Mac/AutoReleasePool.h"
#endif

HS_NS_EDITOR_BEGIN

EditorApplication::EditorApplication(const char* appName) noexcept
    : Application(appName)
    , _guiContext(nullptr)
    , _launcherWindow(nullptr)
    , _deltaTime(0.0f)
{
    _guiContext = new GUIContext();
    ObjectManager::Initialize();
}

EditorApplication::~EditorApplication()
{
    Shutdown();
}

void EditorApplication::Shutdown()
{
    // Close project
    if (ProjectContext::Get().IsProjectOpen())
    {
        ProjectContext::Get().CloseProject();
    }

    if (_window && _window->IsOpened())
    {
        _window->Shutdown();
        delete _window;
        _window = nullptr;
    }

    if (_launcherWindow)
    {
        _launcherWindow->Shutdown();
        delete _launcherWindow;
        _launcherWindow = nullptr;
    }

    if (_guiContext)
    {
        _guiContext->Finalize();
        delete _guiContext;
        _guiContext = nullptr;
    }

    ObjectManager::Finalize();
}

void EditorApplication::Run()
{
    Timer::Start();

    // Check command line for project path
    std::string projectPath = getProjectFromCommandLine();

    // Phase 1: Project Launcher (if no project specified)
    if (projectPath.empty())
    {
        if (!runLauncher())
        {
            // User closed launcher without selecting a project
            return;
        }
        projectPath = _selectedProjectPath;
    }

    // Phase 2: Open Project
    if (!projectPath.empty())
    {
        if (!ProjectContext::Get().OpenProject(projectPath))
        {
            HS_LOG(error, "Failed to open project: {}", projectPath.c_str());
            return;
        }

        // Add to recent projects
        RecentProjects::Get().AddProject(projectPath, ProjectContext::Get().GetProjectName());

        // Initialize AssetDatabase with project asset path
        AssetDatabase::Get().SetRootPath(ProjectContext::Get().GetAssetPath());
        AssetDatabase::Get().Scan();

        // 프로젝트별 레이아웃 경로로 전환
        std::string layoutPath = ProjectContext::Get().GetSettingsPath() + "imgui.ini";
        _guiContext->SetLayoutPath(layoutPath);
    }

    // Phase 3: Editor Window
    runEditor();
}

bool EditorApplication::runLauncher()
{
    EWindowFlags windowFlags = EWindowFlags::None;
    windowFlags |= EWindowFlags::Resizable;
#if defined(__APPLE__)
    windowFlags |= EWindowFlags::Metal;
#else
    windowFlags |= EWindowFlags::Vulkan;
#endif

    _launcherWindow = new ProjectLauncherWindow(this);
    if (nullptr == _launcherWindow->GetNativeWindow().handle)
    {
        HS_LOG(error, "Failed to create launcher window");
        return false;
    }

    ShowNativeWindow(_launcherWindow->GetNativeWindow());

    float lastTime = 0.0f;

    while (_launcherWindow->IsOpened())
    {
#if defined(__APPLE__)
        AutoReleasePool pool;
#endif
        _launcherWindow->ProcessEvent();

        if (!_launcherWindow->IsOpened())
        {
            break;
        }

        // Check if project was selected
        if (_launcherWindow->HasSelectedProject())
        {
            _selectedProjectPath = _launcherWindow->GetSelectedProjectPath();
            break;
        }

        float curTime = Timer::GetElapsedMilliseconds();
        _deltaTime    = (curTime - lastTime) / 1000.0f;
        lastTime      = curTime;

        _launcherWindow->NextFrame();
        _launcherWindow->Update(_deltaTime);
        _launcherWindow->Render();
        _launcherWindow->Present();
        _launcherWindow->Flush();
    }

    // Cleanup launcher
    _launcherWindow->Shutdown();
    delete _launcherWindow;
    _launcherWindow = nullptr;

    return !_selectedProjectPath.empty();
}

void EditorApplication::runEditor()
{
    EWindowFlags windowFlags = EWindowFlags::None;
    windowFlags |= EWindowFlags::Resizable;
    windowFlags |= EWindowFlags::HighPixelDensity;
#if defined(__APPLE__)
    windowFlags |= EWindowFlags::Metal;
#else
    windowFlags |= EWindowFlags::Vulkan;
#endif

    // Create editor window with project name in title
    std::string windowTitle  = "HSMR";
    ERHIPlatform rhiPlatform = RHIContext::Get()->GetCurrentPlatform();
    switch (rhiPlatform)
    {
    case ERHIPlatform::Vulkan:
        windowTitle += " - Vulkan";
        break;
    case ERHIPlatform::Metal:
        windowTitle += " - Metal";
        break;
    case ERHIPlatform::Virtual:
        windowTitle += " - Virtual";
        break;
    default:
        windowTitle += " - Unknown";
        break;
    }
    if (ProjectContext::Get().IsProjectOpen())
    {
        windowTitle += " - " + ProjectContext::Get().GetProjectName();
    }

    _window = new EditorWindow(this, windowTitle.c_str(), 2560, 1440, windowFlags);
    if (nullptr == _window->GetNativeWindow().handle)
    {
        HS_LOG(error, "Failed to initialize editor window");
        return;
    }

    ShowNativeWindow(_window->GetNativeWindow());

    float lastTime = Timer::GetElapsedMilliseconds();

    while (true)
    {
#if defined(__APPLE__)
        AutoReleasePool pool;
#endif
        _window->ProcessEvent();

        if (!_window->IsOpened())
        {
            break;
        }

        float curTime = Timer::GetElapsedMilliseconds();
        _deltaTime    = (curTime - lastTime) / 1000.0f;
        lastTime      = curTime;

        _window->NextFrame();
        _window->Update(_deltaTime);
        _window->Render();
        _window->Present();
        _window->Flush();
    }
}

std::string EditorApplication::getProjectFromCommandLine()
{
    if (!_commandLineProjectPath.empty())
    {
        return _commandLineProjectPath;
    }

    std::string projectPath = CommandLine::GetFlagValue("--project");
    if (!projectPath.empty())
    {
        return projectPath;
    }

    for (const auto& arg : CommandLine::GetArgs())
    {
        if (arg.size() > 7 && arg.substr(arg.size() - 7) == ".hsproj")
        {
            return arg;
        }
    }

    return "";
}

GUIContext* EditorApplication::GetGUIContext()
{
    return _guiContext;
}

HS_NS_EDITOR_END
