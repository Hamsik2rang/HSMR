#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/Core/EngineContext.h"

#include "Engine/RHI/Metal/RHIContextMetal.h"
#include "Engine/RHI/Metal/RHIUtilityMetal.h"
#include "Engine/RHI/Metal/ResourceHandleMetal.h"
#include "Engine/RHI/Metal/RenderHandleMetal.h"
#include "Engine/RHI/Metal/CommandHandleMetal.h"
#include "Engine/RHI/Metal/SwapchainMetal.h"

#include "Engine/Core/Window.h"

#include "ImGui/imgui_impl_sdl3.h"
#include "ImGui/imgui_impl_metal.h"

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
    SwapchainMetal* swMetal      = static_cast<SwapchainMetal*>(swapchain);
    NativeWindow*   nativeHandle = reinterpret_cast<NativeWindow*>(swMetal->nativeHandle);
    
    NSWindow* window = (__bridge_transfer NSWindow*)(nativeHandle->handle);

    MTKView*        mtkView      = (MTKView*)[window contentViewController].view;

    id<MTLDevice> device = ((CAMetalLayer*)mtkView.layer).device;
    ImGui_ImplMetal_Init(device);
}

void BeginRender(Swapchain* swapchain)
{
    @autoreleasepool
    {
        SwapchainMetal*          swMetal = static_cast<SwapchainMetal*>(swapchain);
        MTLRenderPassDescriptor* rpDesc  = static_cast<RenderPassMetal*>(swMetal->GetRenderPass())->handle;

        ImGui_ImplMetal_NewFrame(rpDesc);
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }
}

void EndRender(Swapchain* swapchain)
{
    @autoreleasepool
    {
        CommandBufferMetal* cmdBufferMetal = static_cast<CommandBufferMetal*>(swapchain->GetCommandBufferForCurrentFrame());
        cmdBufferMetal->BeginRenderPass(swapchain->GetRenderPass(), nullptr);

        ImGui::Render();
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cmdBufferMetal->handle, cmdBufferMetal->curRenderEncoder);
    }
}

void FinalizeBackend()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL3_Shutdown();
}

}; // namespace ImGuiExt
