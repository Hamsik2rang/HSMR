#include "Editor/GUI/GUIRenderer.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Renderer/RenderDefinition.h"

#include "Engine/Core/Swapchain.h"
#include "Engine/RHI/RenderHandle.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorWindow.h"

#include "ImGui/imgui.h"
#include "Editor/GUI/ImGuiExtension.h"

HS_NS_EDITOR_BEGIN

GUIRenderer::GUIRenderer(RHIContext* rhiContext)
    : Renderer(rhiContext)
{
}

GUIRenderer::~GUIRenderer()
{
}

//
// void GUIRenderer::Present(Swapchain* swapchain)
//{
//    id<CAMetalDrawable> currentDrawable = (__bridge id<CAMetalDrawable>)swapchain->GetDrawable();
//
//    [_commandBuffer presentDrawable:currentDrawable];
//
//    [_commandBuffer commit];
//}

void GUIRenderer::Shutdown()
{
    ImGuiExt::FinalizeBackend();
    Renderer::Shutdown();
}

HS_NS_EDITOR_END
