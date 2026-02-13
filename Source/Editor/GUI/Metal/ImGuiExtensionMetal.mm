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

// ── Multi-viewport fixes (SDL3 + Metal on macOS) ────────────────────
// Two issues in the bundled imgui backends for secondary viewport windows:
//
// 1) PlatformHandleRaw: Older imgui_impl_sdl3.cpp guards the macOS path
//    behind SDL_VIDEO_DRIVER_COCOA, which SDL3 no longer exports in
//    public headers. If PlatformHandleRaw stays nullptr, the Metal
//    backend interprets the SDL window-ID integer as an NSWindow* → crash.
//    Fix: resolve NSWindow* ourselves via SDL3 properties API.
//
// 2) contentsScale: imgui_impl_metal creates CAMetalLayer with default
//    contentsScale(1.0). On Retina (2x), the first drawable is 1x but
//    ImGui scissor rects are 2x → Metal validation failure.
//    Fix: set contentsScale + drawableSize right after layer creation.
//
// Both are fixed by wrapping ImGuiPlatformIO::Renderer_CreateWindow
// so we never touch third-party source files.
// ─────────────────────────────────────────────────────────────────────
#ifdef __SDL__
static void (*s_originalRendererCreateWindow)(ImGuiViewport*) = nullptr;

// Ensure viewport->PlatformHandleRaw points to the real NSWindow*
static void ImGuiExt_EnsurePlatformHandleRaw(ImGuiViewport* viewport)
{
    if (viewport->PlatformHandleRaw != nullptr)
        return; // already set

    if (viewport->PlatformHandle == nullptr)
        return;

    // PlatformHandle is (void*)(intptr_t)SDL_GetWindowID(window)
    SDL_WindowID window_id = (SDL_WindowID)(intptr_t)viewport->PlatformHandle;
    SDL_Window* sdl_window = SDL_GetWindowFromID(window_id);
    if (sdl_window)
    {
        viewport->PlatformHandleRaw = SDL_GetPointerProperty(
            SDL_GetWindowProperties(sdl_window),
            SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
    }
}

static void ImGuiExt_RendererCreateWindow(ImGuiViewport* viewport)
{
    // Fix PlatformHandleRaw BEFORE the Metal backend reads it
    ImGuiExt_EnsurePlatformHandleRaw(viewport);

    // Call original imgui_impl_metal CreateWindow.
    // Do NOT set contentsScale here — leave it at the default (1.0) so that
    // ImGui_ImplMetal_RenderWindow's own check (contentsScale != fb_scale)
    // fires on the first frame and sets BOTH contentsScale AND drawableSize
    // right before nextDrawable is called.
    if (s_originalRendererCreateWindow)
        s_originalRendererCreateWindow(viewport);
}

// ── RenderWindow wrapper ─────────────────────────────────────────────
// Force correct drawableSize BEFORE the Metal backend calls nextDrawable.
// ImGui_ImplMetal_RenderWindow's own DPI check may be skipped when AppKit
// auto-sets contentsScale on layer-hosted views (Retina displays), and it
// uses window.frame.size (includes title bar) instead of view.bounds.size.
// ─────────────────────────────────────────────────────────────────────
static void (*s_originalRendererRenderWindow)(ImGuiViewport*, void*) = nullptr;

static void ImGuiExt_RendererRenderWindow(ImGuiViewport* viewport, void* renderArg)
{
    void* handle = viewport->PlatformHandleRaw ? viewport->PlatformHandleRaw : viewport->PlatformHandle;
    if (handle)
    {
        NSWindow* window = (__bridge NSWindow*)handle;
        NSView* view = window.contentView;
        CAMetalLayer* layer = (CAMetalLayer*)view.layer;
        if (layer && [layer isKindOfClass:[CAMetalLayer class]])
        {
            float dpiScale = (float)window.backingScaleFactor;
            CGSize viewSize = view.bounds.size;  // points, NOT window.frame.size
            layer.contentsScale = dpiScale;
            layer.drawableSize = CGSizeMake(viewSize.width * dpiScale, viewSize.height * dpiScale);
        }
    }

    if (s_originalRendererRenderWindow)
        s_originalRendererRenderWindow(viewport, renderArg);
}
#endif // __SDL__

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

    // Fix main viewport PlatformHandleRaw (same issue as secondary viewports)
    ImGuiExt_EnsurePlatformHandleRaw(ImGui::GetMainViewport());

    // Hook Renderer_CreateWindow to fix secondary viewports
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    s_originalRendererCreateWindow = platform_io.Renderer_CreateWindow;
    platform_io.Renderer_CreateWindow = ImGuiExt_RendererCreateWindow;

    // Hook Renderer_RenderWindow to fix drawableSize every frame
    s_originalRendererRenderWindow = platform_io.Renderer_RenderWindow;
    platform_io.Renderer_RenderWindow = ImGuiExt_RendererRenderWindow;
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

    MetalSwapchain* swMetal = static_cast<MetalSwapchain*>(swapchain);
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
    ImDrawData* drawData = ImGui::GetDrawData();

    ImGui_ImplMetal_RenderDrawData(drawData, cmdMetalBuffer->handle, cmdMetalBuffer->curRenderEncoder);

    // End the main render encoder before handling additional viewports
    [cmdMetalBuffer->curRenderEncoder endEncoding];
    cmdMetalBuffer->curRenderEncoder = nil;

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
