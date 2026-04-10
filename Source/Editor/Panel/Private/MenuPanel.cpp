#include "Editor/Panel/MenuPanel.h"
#include "Editor/Core/EditorContext.h"
#include "Editor/Project/ProjectContext.h"

#include "Engine/Window.h"
#include "Editor/Core/EditorWindow.h"

#include "Editor/GUI/GUIContext.h"

#include "Editor/Core/EditorApplication.h"

#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"
#include "Core/SystemContext.h"
#include "Core/HAL/FileDialog.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

namespace
{
const char* getPrimaryShortcutLabel(const char* suffix)
{
#if defined(__APPLE__)
    static std::string label;
    label = std::string("Cmd+") + suffix;
    return label.c_str();
#else
    static std::string label;
    label = std::string("Ctrl+") + suffix;
    return label.c_str();
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

    if (ImGui::BeginMenuBar())
    {
        drawFileMenu();
        drawEditMenu();
        drawWindowMenu();
        ImGui::EndMenuBar();
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
            auto* guiContext = static_cast<EditorApplication*>(_window->GetApplication())->GetGUIContext();
            guiContext->SetColorTheme(useWhite);
            useWhite = !useWhite;
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
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false))
        {
            // TODO: Implement undo
        }

        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
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
    auto* guiContext = static_cast<EditorApplication*>(_window->GetApplication())->GetGUIContext();
    if (guiContext)
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
        ImGui::MenuItem("Assets", nullptr, &vis.resources);
        ImGui::MenuItem("Scene Status", nullptr, &vis.sceneStatus);

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
    _currentScenePath.clear();
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
        _currentScenePath = path;
        _sceneDirty = false;
        EditorContext::Get().ClearSelection();
    }
}

void MenuPanel::saveScene()
{
    if (_currentScenePath.empty())
    {
        saveSceneAs();
        return;
    }

    hs::Scene* scene = EditorContext::Get().GetActiveScene();
    if (!scene)
        return;

    hs::SceneSerializer serializer(scene);
    if (serializer.SaveToFile(_currentScenePath))
    {
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
        _currentScenePath = path;
        _sceneDirty = false;
    }
}

HS_NS_EDITOR_END
