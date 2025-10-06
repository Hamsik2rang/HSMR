#include "Editor/Panel/DockspacePanel.h"

#include "ImGui/imgui.h"

HS_NS_EDITOR_BEGIN

DockspacePanel::~DockspacePanel()
{
}

bool DockspacePanel::Setup()
{
    return true;
}

void DockspacePanel::Cleanup()
{
}

void DockspacePanel::Draw()
{
    ImGuiWindowFlags windowFlags      = ImGuiWindowFlags_NoDocking;
    ImGuiDockNodeFlags dockspaceFlags = 0; /*= ImGuiDockNodeFlags_PassthruCentralNode | ImGuiWindowFlags_NoBackground*/

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    dockspaceFlags &= (~ImGuiDockNodeFlags_PassthruCentralNode) ;
    
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    static bool open = true;

    ImGui::Begin("Dockspace Panel", &open, windowFlags);

    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        dockspaceFlags |= ImGuiDockNodeFlags_AutoHideTabBar;
        ImGuiID dockspaceID = ImGui::GetID("Dockspace Panel");
        
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
    }

    for (auto* child : _childs)
    {
        child->Draw();
    }

    ImGui::End();
}

HS_NS_EDITOR_END
