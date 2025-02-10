#include "Editor/Panel/ScenePanel.h"

#include "Editor/GUI/GUIContext.h"

#include "Engine/Renderer/Framebuffer.h"
#include "Engine/Renderer/ResourceHandle.h"
#include "ImGui/imgui.h"

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
    uint32 width = _currentFramebuffer->info.width;
    uint32 height = _currentFramebuffer->info.height;
    
    ImVec2 viewportSize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    Texture* texture = _currentFramebuffer->info.colorBuffers[0];
    
    ImGui::Image(reinterpret_cast<ImTextureID>(texture->handle), viewportSize);
    
    ImGui::End();
}




HS_NS_EDITOR_END
