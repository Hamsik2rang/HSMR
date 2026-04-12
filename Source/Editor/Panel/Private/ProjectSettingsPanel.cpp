#include "Editor/Panel/ProjectSettingsPanel.h"

#include "Editor/Core/EditorContext.h"
#include "Editor/GUI/EditorFeedbackWidgets.h"
#include "Editor/GUI/EditorFormLayout.h"
#include "Editor/Panel/EditorPanelFrame.h"
#include "Editor/Project/ProjectContext.h"

#include "Core/HAL/FileDialog.h"
#include "Scene/Scene.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

ProjectSettingsPanel::ProjectSettingsPanel(Window* window)
    : Panel(window, "Project Settings")
{
}

ProjectSettingsPanel::~ProjectSettingsPanel()
{
}

bool ProjectSettingsPanel::Setup()
{
    return true;
}

void ProjectSettingsPanel::Cleanup()
{
}

void ProjectSettingsPanel::Draw()
{
    if (!IsVisible())
    {
        return;
    }

    EditorPanelWindowOptions panelOptions{};
    panelOptions.pOpen = GetVisibilityBinding();
    EditorPanelFrame::BeginStandardPanel("Project Settings", panelOptions);

    if (!ProjectContext::Get().IsProjectOpen())
    {
        EditorFeedbackWidgets::EmptyState("No project opened.");
        EditorPanelFrame::EndStandardPanel();
        return;
    }

    drawStartupSceneSection();

    EditorPanelFrame::EndStandardPanel();
}

void ProjectSettingsPanel::drawStartupSceneSection()
{
    ProjectContext& projectContext = ProjectContext::Get();
    const ProjectSettings& settings = projectContext.GetSettings();
    const std::string& currentScenePath = EditorContext::Get().GetCurrentScenePath();

    ImGui::TextUnformatted("Startup Scene");
    EditorFeedbackWidgets::SecondaryText(
        "The selected scene is loaded automatically the next time this project opens.");
    ImGui::Separator();

    if (EditorFormLayout::Begin("ProjectStartupSceneForm"))
    {
        EditorFormLayout::BeginRow("Project Path");
        ImGui::TextUnformatted(settings.defaultScene.empty() ? "(Not set)" : settings.defaultScene.c_str());

        EditorFormLayout::BeginRow("Resolved Path");
        const std::string resolvedPath = projectContext.GetResolvedDefaultScenePath();
        ImGui::TextWrapped("%s", resolvedPath.empty() ? "(Not set)" : resolvedPath.c_str());

        EditorFormLayout::BeginRow("Current Scene");
        ImGui::TextWrapped("%s", currentScenePath.empty() ? "(Unsaved scene)" : currentScenePath.c_str());
        EditorFormLayout::End();
    }

    if (ImGui::Button("Browse..."))
    {
        browseStartupScene();
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(currentScenePath.empty());
    if (ImGui::Button("Use Current Scene"))
    {
        setCurrentSceneAsStartup();
    }
    ImGui::EndDisabled();
}

void ProjectSettingsPanel::browseStartupScene()
{
    hs::FileDialogFilter filters[] = {
        {"Scene Files", "*.scene"},
        {"JSON Files", "*.json"},
        {"All Files", "*.*"}
    };

    std::string sceneFolder = ProjectContext::Get().GetScenePath();
    const char* defaultLocation = sceneFolder.empty() ? nullptr : sceneFolder.c_str();
    std::string path = hs::FileDialog::OpenFile(filters, 3, defaultLocation);

    if (path.empty())
    {
        return;
    }

    ProjectContext::Get().SetDefaultScene(path);
}

void ProjectSettingsPanel::setCurrentSceneAsStartup()
{
    const std::string& currentScenePath = EditorContext::Get().GetCurrentScenePath();
    if (currentScenePath.empty())
    {
        return;
    }

    ProjectContext::Get().SetDefaultScene(currentScenePath);
}

HS_NS_EDITOR_END
