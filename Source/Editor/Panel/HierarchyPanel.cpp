#include "Editor/Panel/HierarchyPanel.h"


HS_NS_EDITOR_BEGIN

HierarchyPanel::HierarchyPanel(Window* window)
    : Panel(window)
{

}

bool HierarchyPanel::Setup()
{
	return false;
}

void HierarchyPanel::Cleanup()
{}

void HierarchyPanel::Draw()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("Hierarchy", nullptr);
  
    ImGui::End();

    ImGui::PopStyleVar();
}

HS_NS_EDITOR_END