#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/EngineContext.h"
#include "Engine/Window.h"

#include "RHI/Metal/MetalContext.h"
#include "RHI/Metal/MetalUtility.h"
#include "RHI/Metal/MetalResourceHandle.h"
#include "RHI/Metal/MetalRenderHandle.h"
#include "RHI/Metal/MetalCommandHandle.h"
#include "RHI/Metal/MetalSwapchain.h"

#include "Platform/Mac/MacWindow.h"

#include "ImGui/imgui_impl_metal.h"
#include "ImGui/imgui_impl_osx.h"

using namespace hs;

HS_NS_EDITOR_BEGIN

Swapchain* ImGuiExtension::s_currentSwapchain = nullptr;
uint8 ImGuiExtension::s_currentImageIndex     = 0;

void ImGuiExtension::ImageOffscreen(RHITexture* use_texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
    MetalTexture* texMetal = static_cast<MetalTexture*>(use_texture);

    ImGui::Image(reinterpret_cast<ImTextureID>(texMetal->handle), image_size, uv0, uv1);
}

void ImGuiExtension::InitializeBackend(Swapchain* swapchain)
{
    const auto& nativeWindow = swapchain->GetInfo().nativeWindow;
    NSWindow* window         = (__bridge NSWindow*)(nativeWindow->handle);

    NSView* view        = [(HSViewController*)[window delegate] view];
    CAMetalLayer* layer = (CAMetalLayer*)[view layer];

    id<MTLDevice> device = [layer device];
    ImGui_ImplMetal_Init(device);
    ImGui_ImplOSX_Init(view);
}

void ImGuiExtension::BeginRender(Swapchain* swapchain)
{
    s_currentSwapchain = swapchain;

    SwapchainMetal* swMetal = static_cast<SwapchainMetal*>(swapchain);
    NSWindow* window        = (__bridge NSWindow*)swMetal->nativeHandle;
    HSViewController* vc    = (HSViewController*)[window delegate];
    NSView* view            = [vc view];

    const NativeWindow* nativeWindow = (swapchain->GetInfo().nativeWindow);

    CGSize backingSize       = [vc getBackingViewSize];
    float backingScaleFactor = [window backingScaleFactor];

    ImGuiIO& io                = ImGui::GetIO();
    io.DisplaySize.x           = backingSize.width;
    io.DisplaySize.y           = backingSize.height;
    io.DisplayFramebufferScale = ImVec2(backingScaleFactor, backingScaleFactor);

    MTLRenderPassDescriptor* rpDesc = static_cast<MetalRenderPass*>(swMetal->GetRenderPass())->handle;

    ImGui_ImplMetal_NewFrame(rpDesc);
    ImGui_ImplOSX_NewFrame(view);
    ImGui::NewFrame();
}

void ImGuiExtension::EndRender()
{
    MetalCommandBuffer* cmdMetalBuffer = static_cast<MetalCommandBuffer*>(s_currentSwapchain->GetCommandBufferForCurrentFrame());
    RHIFramebuffer* framebuffer        = s_currentSwapchain->GetFramebufferForCurrentFrame();
    Area area{0, 0, s_currentSwapchain->GetWidth(), s_currentSwapchain->GetHeight()};
    cmdMetalBuffer->BeginRenderPass(s_currentSwapchain->GetRenderPass(), framebuffer, area);

    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), cmdMetalBuffer->handle, cmdMetalBuffer->curRenderEncoder);
}

void ImGuiExtension::FinalizeBackend()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplOSX_Shutdown();
}

void ImGuiExtension::SetProcessEventHandler(void** fnHandler)
{
    // empty.
    // SetNativePreEventHandler(fnHandler);
}

HS_NS_EDITOR_END
