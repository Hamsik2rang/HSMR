#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/Window.h"

#include "Engine/RHI/Metal/RHIContextMetal.h"
#include "Engine/RHI/Metal/RHIUtilityMetal.h"
#include "Engine/RHI/Metal/ResourceHandleMetal.h"
#include "Engine/RHI/Metal/RenderHandleMetal.h"
#include "Engine/RHI/Metal/CommandHandleMetal.h"
#include "Engine/RHI/Metal/SwapchainMetal.h"

#include "Engine/Platform/Mac/PlatformWindowMac.h"

#include "ImGui/imgui_impl_metal.h"
#include "ImGui/imgui_impl_osx.h"

using namespace HS;

namespace ImGuiExt
{
void ImageOffscreen(HS::Texture* use_texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
    TextureMetal* texMetal = static_cast<TextureMetal*>(use_texture);

    ImGui::Image(reinterpret_cast<ImTextureID>(texMetal->handle), image_size, uv0, uv1);
}

void InitializeBackend(Swapchain* swapchain)
{
    const auto& nativeWindow = swapchain->GetInfo().nativeWindow;
    NSWindow* window         = (__bridge NSWindow*)(nativeWindow->handle);

    NSView* view        = [(HSViewController*)[window delegate] view];
    CAMetalLayer* layer = (CAMetalLayer*)[view layer];

    id<MTLDevice> device = [layer device];
    ImGui_ImplMetal_Init(device);
    ImGui_ImplOSX_Init(view);
}

void BeginRender(Swapchain* swapchain)
{
    SwapchainMetal* swMetal = static_cast<SwapchainMetal*>(swapchain);
    NSWindow* window        = (__bridge NSWindow*)swMetal->nativeHandle;
    HSViewController* vc = (HSViewController*)[window delegate];
    NSView* view            = [vc view];

    const NativeWindow* nativeWindow = (swapchain->GetInfo().nativeWindow);
    
    CGSize backingSize = [vc getBackingViewSize];
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = backingSize.width;
    io.DisplaySize.y = backingSize.height;
    
    HS_LOG(info, "io DisplaySize: %f, %f", io.DisplaySize.x, io.DisplaySize.y);
    
    MTLRenderPassDescriptor* rpDesc = static_cast<RenderPassMetal*>(swMetal->GetRenderPass())->handle;

    ImGui_ImplMetal_NewFrame(rpDesc);
    ImGui_ImplOSX_NewFrame(view);
    ImGui::NewFrame();
}

void EndRender(Swapchain* swapchain)
{
    CommandBufferMetal* cmdBufferMetal = static_cast<CommandBufferMetal*>(swapchain->GetCommandBufferForCurrentFrame());
    cmdBufferMetal->BeginRenderPass(swapchain->GetRenderPass(), nullptr);

    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cmdBufferMetal->handle, cmdBufferMetal->curRenderEncoder);
}

void FinalizeBackend()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplOSX_Shutdown();
}

}; // namespace ImGuiExt
