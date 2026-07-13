#include "Editor/Panel/MenuPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Project/ProjectContext.h"

#include "Engine/Window.h"
#include "Editor/Core/EditorWindow.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/Panel/EditorPanelFrame.h"

#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"
#include "Core/SystemContext.h"
#include "Core/HAL/FileDialog.h"

#include "ImGui/imgui.h"

#include <cstdio>

HS_NS_EDITOR_BEGIN

namespace
{
const char* getPrimaryShortcutLabel(const char* suffix)
{
    static thread_local char label[64];
#if defined(__APPLE__)
    std::snprintf(label, sizeof(label), "Cmd+%s", suffix);
#else
    std::snprintf(label, sizeof(label), "Ctrl+%s", suffix);
#endif
    return label;
}

const char* getRedoShortcutLabel()
{
#if defined(__APPLE__)
    return getPrimaryShortcutLabel("Shift+Z");
#else
    return getPrimaryShortcutLabel("Y");
#endif
}
}

bool MenuPanel::Setup()
{
    return true;
}

void MenuPanel::Cleanup()
{
}

void MenuPanel::Draw()
{
    static bool showDemo = false;

    const bool isMainMenuBar = (_mode == EMode::MainMenuBar);
    const bool opened = isMainMenuBar
                      ? ImGui::BeginMainMenuBar()
                      : EditorPanelFrame::BeginPanelMenuBar();

    if (opened)
    {
        drawFileMenu();
        drawEditMenu();
        drawWindowMenu();

        if (isMainMenuBar)
        {
            ImGui::EndMainMenuBar();
        }
        else
        {
            EditorPanelFrame::EndPanelMenuBar();
        }
    }

    if (showDemo)
    {
        ImGui::ShowDemoWindow(&showDemo);
    }
}

void MenuPanel::drawFileMenu()
{
    static bool useWhite = false;
    static bool showDemo = false;

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New Scene", getPrimaryShortcutLabel("N")))
        {
            newScene();
        }

        if (ImGui::MenuItem("Open Scene...", getPrimaryShortcutLabel("O")))
        {
            openScene();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save Scene", getPrimaryShortcutLabel("S")))
        {
            saveScene();
        }

        if (ImGui::MenuItem("Save Scene As...", getPrimaryShortcutLabel("Shift+S")))
        {
            saveSceneAs();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Show Demo", nullptr, &showDemo))
        {
        }

        if (ImGui::MenuItem("Change Theme", nullptr, false))
        {
            if (auto* guiContext = _window->GetGUIContext())
            {
                guiContext->SetColorTheme(useWhite);
                useWhite = !useWhite;
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Exit", "Alt+F4"))
        {
            _window->Close();
        }

        ImGui::EndMenu();
    }

    if (showDemo)
    {
        ImGui::ShowDemoWindow(&showDemo);
    }
}

void MenuPanel::drawEditMenu()
{
    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Undo", getPrimaryShortcutLabel("Z"), false, false))
        {
            // TODO: Implement undo
        }

        if (ImGui::MenuItem("Redo", getRedoShortcutLabel(), false, false))
        {
            // TODO: Implement redo
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Save Layout"))
        {
            ExecuteSaveLayout();
        }

        ImGui::EndMenu();
    }
}

void MenuPanel::ExecuteSaveLayout()
{
    if (auto* guiContext = _window->GetGUIContext())
    {
        guiContext->SaveLayout("");
    }
}

void MenuPanel::drawWindowMenu()
{
    if (ImGui::BeginMenu("Window"))
    {
        auto& vis = EditorContext::Get().GetPanelVisibility();

        ImGui::MenuItem("Scene", nullptr, &vis.scene);
        ImGui::MenuItem("Game", nullptr, &vis.game);
        ImGui::MenuItem("Hierarchy", nullptr, &vis.hierarchy);
        ImGui::MenuItem("Inspector", nullptr, &vis.inspector);

        ImGui::Separator();

        ImGui::MenuItem("Profiler", nullptr, &vis.profiler);

        ImGui::EndMenu();
    }
}

void MenuPanel::newScene()
{
    hs::Scene* scene = EditorContext::Get().GetActiveScene();
    if (!scene)
        return;

    // Clear and create new scene
    hs::SceneSerializer serializer(scene);
    serializer.ClearScene();

    scene->SetName("New Scene");
    EditorContext::Get().ClearCurrentScenePath();
    _sceneDirty = false;

    EditorContext::Get().ClearSelection();
}

void MenuPanel::openScene()
{
    hs::FileDialogFilter filters[] = {
        {"Scene Files", "*.scene"},
        {"JSON Files", "*.json"},
        {"All Files", "*.*"}
    };

    const char* defaultLocation = nullptr;
    std::string sceneFolder;
    if (ProjectContext::Get().IsProjectOpen())
    {
        sceneFolder = ProjectContext::Get().GetScenePath();
        defaultLocation = sceneFolder.c_str();
    }

    std::string path = hs::FileDialog::OpenFile(filters, 3, defaultLocation);

    if (path.empty())
        return;

    hs::Scene* scene = EditorContext::Get().GetActiveScene();
    if (!scene)
        return;

    hs::SceneSerializer serializer(scene);
    if (serializer.LoadFromFile(path))
    {
        EditorContext::Get().SetCurrentScenePath(path);
        updateStartupScene(path);
        _sceneDirty = false;
        EditorContext::Get().ClearSelection();
    }
}

void MenuPanel::saveScene()
{
    const std::string& currentScenePath = EditorContext::Get().GetCurrentScenePath();
    if (currentScenePath.empty())
    {
        saveSceneAs();
        return;
    }

    hs::Scene* scene = EditorContext::Get().GetActiveScene();
    if (!scene)
        return;

    hs::SceneSerializer serializer(scene);
    if (serializer.SaveToFile(currentScenePath))
    {
        updateStartupScene(currentScenePath);
        _sceneDirty = false;
    }
}

void MenuPanel::saveSceneAs()
{
    hs::FileDialogFilter filters[] = {
        {"Scene Files", "*.scene"},
        {"JSON Files", "*.json"},
        {"All Files", "*.*"}
    };

    const char* defaultLocation = nullptr;
    std::string sceneFolder;
    if (ProjectContext::Get().IsProjectOpen())
    {
        sceneFolder = ProjectContext::Get().GetScenePath();
        defaultLocation = sceneFolder.c_str();
    }

    std::string path = hs::FileDialog::SaveFile(filters, 3, defaultLocation);

    if (path.empty())
        return;

    hs::Scene* scene = EditorContext::Get().GetActiveScene();
    if (!scene)
        return;

    hs::SceneSerializer serializer(scene);
    if (serializer.SaveToFile(path))
    {
        EditorContext::Get().SetCurrentScenePath(path);
        updateStartupScene(path);
        _sceneDirty = false;
    }
}

void MenuPanel::updateStartupScene(const std::string& scenePath)
{
    if (scenePath.empty() || !ProjectContext::Get().IsProjectOpen())
    {
        return;
    }

    ProjectContext::Get().SetDefaultScene(scenePath);
}

HS_NS_EDITOR_END
