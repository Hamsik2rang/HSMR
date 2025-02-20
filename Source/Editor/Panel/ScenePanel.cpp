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
    
}

void ScenePanel::Cleanup()
{
    
}

void ScenePanel::Draw()
{
    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse);
    
    ImGui::SetScrollY(0.0f);
    uint32 width = _currentRenderTarget->GetWidth();
    uint32 height = _currentRenderTarget->GetHeight();
    
    ImVec2 viewportSize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    Texture* texture = _currentRenderTarget->GetColorTexture(0);
    
    ImGuiExt::ImageOffscreen(texture, viewportSize);

    
    
    ImGui::End();
}




HS_NS_EDITOR_END
