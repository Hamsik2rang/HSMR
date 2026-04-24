//
//  ProjectLauncherWindow.cpp
//  Editor
//
//  Created by Claude on 2/10/26.
//

#include "Editor/Project/ProjectLauncherWindow.h"
#include "Editor/Project/ProjectContext.h"
#include "Editor/Project/RecentProjects.h"

#include "Editor/GUI/EditorDialogFrame.h"
#include "Editor/GUI/EditorFeedbackWidgets.h"
#include "Editor/GUI/GUIContext.h"
#include "Editor/GUI/EditorFormLayout.h"
#include "Editor/GUI/EditorListWidgets.h"
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

namespace
{
ImVec4 lerpColor(const ImVec4& lhs, const ImVec4& rhs, float t)
{
    return ImVec4(
        lhs.x + (rhs.x - lhs.x) * t,
        lhs.y + (rhs.y - lhs.y) * t,
        lhs.z + (rhs.z - lhs.z) * t,
        lhs.w + (rhs.w - lhs.w) * t
    );
}
}

ProjectLauncherWindow::ProjectLauncherWindow(Application* ownerApp)
    : Window(ownerApp, "HSMR", 1024, 600,
#if defined(__APPLE__)
             EWindowFlags::Resizable | EWindowFlags::Metal)
#else
             EWindowFlags::Resizable | EWindowFlags::Vulkan)
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
    auto* sysContext = SystemContext::Get();
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
        if (_rhiContext->GetCurrentPlatform() == ERHIPlatform::Vulkan)
        {
            titleText += " - Vulkan";
        }
        else if (_rhiContext->GetCurrentPlatform() == ERHIPlatform::Metal)
        {
            titleText += " - Metal";
        }

        // Logo/Title area
        ImGui::PushFont(nullptr); // Use default font for now
        ImGui::SetCursorPosY(20);
        ImGui::SetCursorPosX(20);
        ImGui::TextColored(ImVec4(0.12f, 0.36f, 0.72f, 1.0f), "%s", titleText.c_str());
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
        EditorFeedbackWidgets::SecondaryText("Version 0.1.0");
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
    const ImVec4 buttonColor = EditorListWidgets::GetSurfaceColor();
    const ImVec4 buttonHoveredColor = EditorListWidgets::GetSurfaceHoverColor();
    const ImVec4 textColor = EditorListWidgets::GetPrimaryTextColor();
    const ImVec4 textDisabledColor = EditorListWidgets::GetSecondaryTextColor();
    const ImVec4 warningColor = EditorListWidgets::GetTintedSurfaceColor(ImVec4(0.85f, 0.25f, 0.25f, 1.0f), false);
    const ImVec4 warningHoveredColor = EditorListWidgets::GetTintedSurfaceColor(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), true);

    if (recentProjects.empty())
    {
        ImGui::SetCursorPos(ImVec2(20, 80));
        EditorFeedbackWidgets::EmptyState(
            "No recent projects",
            "Create a new project or open an existing one.");
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

        ImVec4 bgColor = isHovered ? buttonHoveredColor : buttonColor;
        if (!project.exists)
        {
            bgColor = isHovered ? warningHoveredColor : warningColor;
        }

        ImGui::GetWindowDrawList()->AddRectFilled(
            cardStart,
            ImVec2(cardStart.x + cardSize.x, cardStart.y + cardSize.y),
            ImGui::ColorConvertFloat4ToU32(bgColor),
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
                std::string folder = FileSystem::GetDirectory(project.path);
                FileDialog::OpenInExplorer(folder);
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
        ImGui::TextColored(textColor, "%s", project.name.c_str());

        // Project path
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x + 15, cardStart.y + 32));
        if (project.exists)
        {
            EditorFeedbackWidgets::SecondaryText(project.path.c_str());
        }
        else
        {
            ImGui::TextColored(lerpColor(textDisabledColor, ImVec4(1.0f, 0.2f, 0.2f, 1.0f), 0.65f), "(Project not found)");
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
            EditorFeedbackWidgets::SecondaryText(timeStr);
        }

        ImGui::PopID();

        // Move cursor for next card
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x, cardStart.y + cardSize.y + 10));
    }
}

void ProjectLauncherWindow::drawNewProjectDialog()
{
    if (EditorDialogFrame::BeginCenteredModal("New Project", &_showNewProjectDialog, ImVec2(600, 300)))
    {
        ImGui::Spacing();

        if (EditorFormLayout::Begin("NewProjectForm", 110.0f))
        {
            EditorFormLayout::InputTextRow("Project Name", _newProjectNameBuffer, sizeof(_newProjectNameBuffer));

            EditorFormLayout::BeginRow("Location");
            const float browseButtonWidth = 80.0f;
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - browseButtonWidth - spacing);
            ImGui::InputText("##Location", _newProjectPathBuffer, sizeof(_newProjectPathBuffer));
            ImGui::SameLine();
            if (ImGui::Button("Browse...", ImVec2(browseButtonWidth, 0)))
            {
                std::string folder = FileDialog::OpenFolder();
                if (!folder.empty())
                {
                    strncpy(_newProjectPathBuffer, folder.c_str(), sizeof(_newProjectPathBuffer) - 1);
                    _newProjectPathBuffer[sizeof(_newProjectPathBuffer) - 1] = '\0';
                }
            }

            EditorFormLayout::BeginRow("Full Path");
            std::string fullPathPreview = std::string(_newProjectPathBuffer);
            if (!fullPathPreview.empty() && fullPathPreview.back() != HS_DIR_SEPERATOR)
            {
                fullPathPreview += HS_DIR_SEPERATOR;
            }
            fullPathPreview += _newProjectNameBuffer;
            EditorFeedbackWidgets::SecondaryText(fullPathPreview.c_str());

            EditorFormLayout::End();
        }

        // Full path preview
        ImGui::Spacing();
        std::string fullPath = std::string(_newProjectPathBuffer);
        if (!fullPath.empty() && fullPath.back() != HS_DIR_SEPERATOR)
        {
            fullPath += HS_DIR_SEPERATOR;
        }
        fullPath += _newProjectNameBuffer;
        //ImGui::TextDisabled("Project will be created at: %s", fullPath.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Buttons
        const float buttonWidth = 120.0f;
        EditorDialogFrame::BeginFooterButtons(2, buttonWidth, 10.0f);

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

        EditorDialogFrame::EndModal();
    }
}

void ProjectLauncherWindow::drawOpenProjectDialog()
{
    FileDialogFilter filters[] = {
        {"HSMR Project", "*.hsproj"},
        {"All Files", "*.*"}
    };

    std::string path = FileDialog::OpenFile(filters, 2);
    if (!path.empty() && FileSystem::Exist(path))
    {
        _selectedProjectPath = path;

        // Extract project name from path
        std::string name = FileSystem::GetFileNameWithoutExtension(path);
        RecentProjects::Get().AddProject(path, name);
    }
}

HS_NS_EDITOR_END
