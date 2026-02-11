#include "Editor/GUI/ImGuiExtension.h"

#include "Engine/EngineContext.h"
#include "Engine/Window.h"

#include "RHI/Metal/MetalContext.h"
#include "RHI/Metal/MetalUtility.h"
#include "RHI/Metal/MetalResourceHandle.h"
#include "RHI/Metal/MetalRenderHandle.h"
#include "RHI/Metal/MetalCommandHandle.h"
#include "RHI/Metal/MetalSwapchain.h"

#include "Core/Native/NativeWindow.h"

#ifdef __SDL__
#include <SDL3/SDL.h>
#include "ImGui/imgui_impl_sdl3.h"
#include "ImGui/imgui_impl_metal.h"
#else
#include "Platform/Mac/MacWindow.h"
#include "ImGui/imgui_impl_metal.h"
#include "ImGui/imgui_impl_osx.h"
#endif

#import <QuartzCore/CAMetalLayer.h>

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

#ifdef __SDL__
    // === SDL3 + Metal path ===
    SDL_Window* window = (SDL_Window*)(nativeWindow->handle);
    CAMetalLayer* layer = (__bridge CAMetalLayer*)(nativeWindow->graphicsLayer);
    id<MTLDevice> device = [layer device];

    ImGui_ImplMetal_Init(device);
    ImGui_ImplSDL3_InitForMetal(window);
#else
    // === Native macOS + Metal path ===
    NSWindow* window = (__bridge NSWindow*)(nativeWindow->handle);
    NSView* view = (__bridge NSView*)(nativeWindow->graphicsView);
    CAMetalLayer* layer = (__bridge CAMetalLayer*)(nativeWindow->graphicsLayer);

    id<MTLDevice> device = [layer device];
    ImGui_ImplMetal_Init(device);
    ImGui_ImplOSX_Init(view);
#endif
}

void ImGuiExtension::BeginRender(Swapchain* swapchain)
{
    s_currentSwapchain = swapchain;

    SwapchainMetal* swMetal = static_cast<SwapchainMetal*>(swapchain);
    const NativeWindow* nativeWindow = swapchain->GetInfo().nativeWindow;

    MTLRenderPassDescriptor* rpDesc = static_cast<MetalRenderPass*>(swMetal->GetRenderPass())->handle;

#ifdef __SDL__
    // === SDL3 path ===
    SDL_Window* window = (SDL_Window*)(nativeWindow->handle);

    // Get drawable size from SDL
    int drawableWidth, drawableHeight;
    SDL_GetWindowSizeInPixels(window, &drawableWidth, &drawableHeight);

    float displayScale = SDL_GetWindowDisplayScale(window);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(drawableWidth);
    io.DisplaySize.y = static_cast<float>(drawableHeight);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f); // SDL already handles scaling

    ImGui_ImplMetal_NewFrame(rpDesc);
    ImGui_ImplSDL3_NewFrame();
#else
    // === Native macOS path ===
    NSWindow* window = (__bridge NSWindow*)swMetal->nativeHandle;
    HSViewController* vc = (HSViewController*)[window delegate];
    NSView* view = [vc view];

    CGSize backingSize = [vc getBackingViewSize];
    float backingScaleFactor = [window backingScaleFactor];

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = backingSize.width;
    io.DisplaySize.y = backingSize.height;
    io.DisplayFramebufferScale = ImVec2(backingScaleFactor, backingScaleFactor);

    ImGui_ImplMetal_NewFrame(rpDesc);
    ImGui_ImplOSX_NewFrame(view);
#endif

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

    // Update and render additional platform windows (multi-viewport)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void ImGuiExtension::FinalizeBackend()
{
    ImGui_ImplMetal_Shutdown();
#ifdef __SDL__
    ImGui_ImplSDL3_Shutdown();
#else
    ImGui_ImplOSX_Shutdown();
#endif
}

void ImGuiExtension::SetProcessEventHandler(void** fnHandler)
{
#ifdef __SDL__
    *fnHandler = reinterpret_cast<void*>(ImGui_ImplSDL3_ProcessEvent);
#else
    // Native macOS path - empty
#endif
}

HS_NS_EDITOR_END
