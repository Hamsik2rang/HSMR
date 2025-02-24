#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/Core/EngineContext.h"

#include "Engine/RHI/Metal/RHIContextMetal.h"
#include "Engine/RHI/Metal/RHIUtilityMetal.h"
#include "Engine/RHI/Metal/ResourceHandleMetal.h"
#include "Engine/RHI/Metal/RenderHandleMetal.h"
#include "Engine/RHI/Metal/CommandHandleMetal.h"
#include "Engine/RHI/Metal/SwapchainMetal.h"

#include "Engine/Core/Window.h"

#include "ImGui/backends/imgui_impl_sdl3.h"
#include "ImGui/backends/imgui_impl_metal.h"

using namespace HS;

namespace ImGuiExt
{
void ImageOffscreen(HS::Texture* use_texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
}

void InitializeBackend(Swapchain* swapchain)
{
    SwapchainMetal*     swMetal      = static_cast<SwapchainMetal*>(swapchain);
    NativeWindowHandle* nativeHandle = reinterpret_cast<NativeWindowHandle*>(swMetal->nativeHandle);

    id<MTLDevice> device = ((__bridge_transfer CAMetalLayer*)nativeHandle->layer).device;
    ImGui_ImplMetal_Init(device);
    ImGui_ImplSDL3_InitForMetal(static_cast<SDL_Window*>(nativeHandle->window));
}

void BeginRender(Swapchain* swapchain)
{
    SwapchainMetal*          swMetal = static_cast<SwapchainMetal*>(swapchain);
    MTLRenderPassDescriptor* rpDesc  = static_cast<RenderPassMetal*>(swMetal->GetRenderPass())->handle;

    ImGui_ImplMetal_NewFrame(rpDesc);
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void EndRender(Swapchain* swapchain)
{
    CommandBufferMetal* cmdBufferMetal = static_cast<CommandBufferMetal*>(swapchain->GetCommandBufferForCurrentFrame());
    cmdBufferMetal->BeginRenderPass(swapchain->GetRenderPass(), nullptr);

    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cmdBufferMetal->handle, cmdBufferMetal->curRenderEncoder);

    ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void FinalizeBackend()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL3_Shutdown();
}

}; // namespace ImGuiExt
