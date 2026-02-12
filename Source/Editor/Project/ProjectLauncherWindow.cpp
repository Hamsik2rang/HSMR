//
//  ProjectLauncherWindow.cpp
//  Editor
//
//  Created by Claude on 2/10/26.
//

#include "Editor/Project/ProjectLauncherWindow.h"
#include "Editor/Project/ProjectContext.h"
#include "Editor/Project/RecentProjects.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/GUI/ImGuiExtension.h"
#include "Editor/Core/EditorApplication.h"

#include "Core/SystemContext.h"
#include "Core/HAL/FileDialog.h"
#include "Core/HAL/FileSystem.h"
#include "Core/Log.h"

#include "RHI/Swapchain.h"

#include "ImGui/imgui.h"

#include <chrono>
#include <ctime>

HS_NS_EDITOR_BEGIN

ProjectLauncherWindow::ProjectLauncherWindow(Application* ownerApp)
    : Window(ownerApp, "HSMR", 1024, 600,
#if defined(__APPLE__)
             EWindowFlags::WINDOW_RESIZABLE | EWindowFlags::WINDOW_METAL)
#else
             EWindowFlags::WINDOW_RESIZABLE | EWindowFlags::WINDOW_VULKAN)
#endif
{
    onInitialize();
}

ProjectLauncherWindow::~ProjectLauncherWindow()
{
}

bool ProjectLauncherWindow::onInitialize()
{
    // Get GUI context from application
    _guiContext = static_cast<EditorApplication*>(_ownerApp)->GetGUIContext();
    float dpiScale = _nativeWindow.scale;
    if (dpiScale > 1.0f)
    {
        _guiContext->ApplyDPIScale(dpiScale);
    }

    ImGuiExtension::InitializeBackend(_swapchain);

    void* handler = nullptr;
    ImGuiExtension::SetProcessEventHandler(&handler);
    SetPreEventHandler(handler);

    // Load recent projects
    RecentProjects::Get().Load();

    // Set default project path to Documents
    auto* sysContext = hs::SystemContext::Get();
    if (sysContext && !sysContext->userDocumentsDir.empty())
    {
        strncpy(_newProjectPathBuffer, sysContext->userDocumentsDir.c_str(),
                sizeof(_newProjectPathBuffer) - 1);
    }

    return true;
}

void ProjectLauncherWindow::onNextFrame()
{
    if (!_shouldPresent)
    {
        return;
    }

    _rhiContext->AcquireNextImage(_swapchain);
}

void ProjectLauncherWindow::onUpdate(float deltaTime)
{
    // Nothing to update
}

void ProjectLauncherWindow::onRender()
{
    if (!_shouldPresent)
    {
        return;
    }

    RHICommandBuffer* cmdBuffer = _swapchain->GetCommandBufferForCurrentFrame();
    cmdBuffer->Begin();

    _guiContext->BeginRender(_swapchain);
    drawLauncherUI();
    _guiContext->EndRender();

    cmdBuffer->End();

    _rhiContext->Submit(_swapchain, &cmdBuffer, 1);
}

void ProjectLauncherWindow::onPresent()
{
    if (!_shouldPresent)
    {
        return;
    }
    RHIContext::Get()->Present(_swapchain);
}

void ProjectLauncherWindow::onShutdown()
{
    ImGuiExtension::FinalizeBackend();
}

void ProjectLauncherWindow::drawLauncherUI()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration |
                                   ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("##Launcher", nullptr, windowFlags);

    // Left sidebar - Actions
    ImGui::BeginChild("Actions", ImVec2(220, 0), true);
    {
        std::string titleText = "HSMR";
        if (_rhiContext->GetCurrentPlatform() == ERHIPlatform::VULKAN)
        {
            titleText += " - Vulkan";
        }
        else if (_rhiContext->GetCurrentPlatform() == ERHIPlatform::METAL)
        {
            titleText += " - Metal";
        }

        // Logo/Title area
        ImGui::PushFont(nullptr); // Use default font for now
        ImGui::SetCursorPosY(20);
        ImGui::SetCursorPosX(20);
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), titleText.c_str());
        ImGui::PopFont();

        ImGui::SetCursorPosY(50);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        // New Project button
        ImGui::SetCursorPosX(10);
        if (ImGui::Button("New Project", ImVec2(200, 45)))
        {
            _showNewProjectDialog = true;
            memset(_newProjectNameBuffer, 0, sizeof(_newProjectNameBuffer));
        }

        ImGui::Spacing();

        // Open Project button
        ImGui::SetCursorPosX(10);
        if (ImGui::Button("Open Project...", ImVec2(200, 45)))
        {
            drawOpenProjectDialog();
        }

        // Bottom area - Version info
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::SetCursorPosX(10);
        ImGui::TextDisabled("Version 0.1.0");
        ImGui::SetCursorPosX(10);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Right panel - Recent Projects
    ImGui::BeginChild("RecentProjects", ImVec2(0, 0), true);
    {
        ImGui::SetCursorPos(ImVec2(20, 20));
        ImGui::Text("Recent Projects");

        ImGui::SetCursorPosY(45);
        ImGui::Separator();
        ImGui::Spacing();

        drawProjectList();
    }
    ImGui::EndChild();

    ImGui::End();

    // New Project Dialog
    if (_showNewProjectDialog)
    {
        drawNewProjectDialog();
    }
}

void ProjectLauncherWindow::drawProjectList()
{
    auto& recentProjects = RecentProjects::Get().GetProjects();

    if (recentProjects.empty())
    {
        ImGui::SetCursorPos(ImVec2(20, 80));
        ImGui::TextDisabled("No recent projects");
        ImGui::SetCursorPosX(20);
        ImGui::TextDisabled("Create a new project or open an existing one.");
        return;
    }

    ImGui::SetCursorPosX(10);

    for (size_t i = 0; i < recentProjects.size(); ++i)
    {
        const auto& project = recentProjects[i];

        ImGui::PushID(static_cast<int>(i));

        // Project card
        ImVec2 cardStart = ImGui::GetCursorScreenPos();
        ImVec2 cardSize(ImGui::GetContentRegionAvail().x - 20, 80);

        // Hover effect
        ImVec2 mousePos  = ImGui::GetMousePos();
        bool isHovered   = mousePos.x >= cardStart.x && mousePos.x <= cardStart.x + cardSize.x &&
                           mousePos.y >= cardStart.y && mousePos.y <= cardStart.y + cardSize.y;

        ImU32 bgColor = isHovered ? IM_COL32(60, 60, 70, 255) : IM_COL32(45, 45, 55, 255);
        if (!project.exists)
        {
            bgColor = IM_COL32(70, 40, 40, 255);
        }

        ImGui::GetWindowDrawList()->AddRectFilled(
            cardStart,
            ImVec2(cardStart.x + cardSize.x, cardStart.y + cardSize.y),
            bgColor,
            5.0f
        );

        // Invisible button for click detection
        ImGui::SetCursorScreenPos(cardStart);
        if (ImGui::InvisibleButton("##card", cardSize))
        {
            if (project.exists)
            {
                _selectedProjectPath = project.path;
            }
        }

        // Right-click context menu (must be right after InvisibleButton so it's the "last item")
        if (ImGui::BeginPopupContextItem("##cardcontext"))
        {
            if (ImGui::MenuItem("Open in Explorer"))
            {
                std::string folder = hs::FileSystem::GetDirectory(project.path);
#ifdef _WIN32
                ShellExecuteA(nullptr, "explore", folder.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
            }
            if (ImGui::MenuItem("Remove from List"))
            {
                RecentProjects::Get().RemoveProject(project.path);
            }
            ImGui::EndPopup();
        }

        // Card content
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x + 15, cardStart.y + 12));

        // Project name
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", project.name.c_str());

        // Project path
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x + 15, cardStart.y + 32));
        if (project.exists)
        {
            ImGui::TextDisabled("%s", project.path.c_str());
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "(Project not found)");
        }

        // Last opened time
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x + 15, cardStart.y + 50));
        if (project.lastOpened > 0)
        {
            time_t time = static_cast<time_t>(project.lastOpened);
            char timeStr[64];
            struct tm timeInfo;
#ifdef _WIN32
            localtime_s(&timeInfo, &time);
#else
            localtime_r(&time, &timeInfo);
#endif
            strftime(timeStr, sizeof(timeStr), "Last opened: %Y-%m-%d %H:%M", &timeInfo);
            ImGui::TextDisabled("%s", timeStr);
        }

        ImGui::PopID();

        // Move cursor for next card
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x, cardStart.y + cardSize.y + 10));
    }
}

void ProjectLauncherWindow::drawNewProjectDialog()
{
    ImGui::OpenPopup("New Project");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(550, 220));

    if (ImGui::BeginPopupModal("New Project", &_showNewProjectDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Spacing();

        // Project Name
        ImGui::Text("Project Name:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##name", _newProjectNameBuffer, sizeof(_newProjectNameBuffer));

        ImGui::Spacing();
        ImGui::Spacing();

        // Location
        ImGui::Text("Location:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 90);
        ImGui::InputText("##path", _newProjectPathBuffer, sizeof(_newProjectPathBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Browse...", ImVec2(80, 0)))
        {
            std::string folder = hs::FileDialog::OpenFolder();
            if (!folder.empty())
            {
                strncpy(_newProjectPathBuffer, folder.c_str(), sizeof(_newProjectPathBuffer) - 1);
                _newProjectPathBuffer[sizeof(_newProjectPathBuffer) - 1] = '\0';
            }
        }

        // Full path preview
        ImGui::Spacing();
        std::string fullPath = std::string(_newProjectPathBuffer);
        if (!fullPath.empty() && fullPath.back() != HS_DIR_SEPERATOR)
        {
            fullPath += HS_DIR_SEPERATOR;
        }
        fullPath += _newProjectNameBuffer;
        ImGui::TextDisabled("Project will be created at: %s", fullPath.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Buttons
        float buttonWidth = 120;
        float spacing     = 10;
        float totalWidth  = buttonWidth * 2 + spacing;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - totalWidth) * 0.5f);

        bool canCreate = strlen(_newProjectNameBuffer) > 0 && strlen(_newProjectPathBuffer) > 0;

        if (!canCreate)
        {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("Create", ImVec2(buttonWidth, 35)))
        {
            _newProjectName    = _newProjectNameBuffer;
            _newProjectPathStr = fullPath;
            _createNewProject  = true;
            _showNewProjectDialog = false;

            // Create project
            if (ProjectContext::Get().CreateProject(fullPath, _newProjectName))
            {
                _selectedProjectPath = ProjectContext::Get().GetProjectFilePath();
                RecentProjects::Get().AddProject(_selectedProjectPath, _newProjectName);
            }
        }

        if (!canCreate)
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 35)))
        {
            _showNewProjectDialog = false;
        }

        ImGui::EndPopup();
    }
}

void ProjectLauncherWindow::drawOpenProjectDialog()
{
    hs::FileDialogFilter filters[] = {
        {"HSMR Project", "*.hsproj"},
        {"All Files", "*.*"}
    };

    std::string path = hs::FileDialog::OpenFile(filters, 2);
    if (!path.empty() && hs::FileSystem::Exist(path))
    {
        _selectedProjectPath = path;

        // Extract project name from path
        std::string name = hs::FileSystem::GetFileNameWithoutExtension(path);
        RecentProjects::Get().AddProject(path, name);
    }
}

HS_NS_EDITOR_END
