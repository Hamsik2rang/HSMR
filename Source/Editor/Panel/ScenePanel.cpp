#include "Editor/Panel/ScenePanel.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/RHI/RenderHandle.h"
#include "Engine/RHI/ResourceHandle.h"


HS_NS_EDITOR_BEGIN

ScenePanel::~ScenePanel()
{
    
}

bool ScenePanel::Setup()
{
    return true;
}

void ScenePanel::Cleanup()
{
    
}

void ScenePanel::Draw()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    
    ImGui::SetScrollY(0.0f);
    uint32 width = _currentRenderTarget->GetWidth();
    uint32 height = _currentRenderTarget->GetHeight();
    
    ImVec2 viewportSize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    Texture* texture = _currentRenderTarget->GetColorTexture(0);
    
    ImGuiExt::ImageOffscreen(texture, viewportSize);

    ImVec2 curPanelSize = ImGui::GetWindowSize();
    _resolution.width = static_cast<uint32>(curPanelSize.x);
    _resolution.height = static_cast<uint32>(curPanelSize.y);
    
    ImGui::End();
    
    ImGui::PopStyleVar();
}


HS_NS_EDITOR_END
