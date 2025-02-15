#include "Editor/GUI/GUIRenderer.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Renderer/RenderDefinition.h"
#include "Engine/Renderer/RHIUtility.h"
#include "Engine/Renderer/Swapchain.h"
#include "Engine/Renderer/RenderPass.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorWindow.h"

#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_sdl3.h"
#include "ImGui/backends/imgui_impl_metal.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

HS_NS_EDITOR_BEGIN

#define METAL_LAYER(x)         ((__bridge CAMetalLayer*)(x))
#define METAL_DEVICE(x)        ((__bridge id<MTLDevice>)(x))
#define METAL_COMMAND_QUEUE(x) ((__bridge id<MTLCommandQueue>)(x))

namespace
{
id<MTLCommandBuffer>        _commandBuffer;
id<MTLRenderCommandEncoder> _commandEncoder;

id<MTLTexture>           _renderTarget[Renderer::MAX_SUBMIT_INDEX];
id<CAMetalDrawable>      _currentDrawable;
//MTLRenderPassDescriptor* _renderPassDescriptor;
} // namespace

GUIRenderer::GUIRenderer()
{
}

GUIRenderer::~GUIRenderer()
{
}

bool GUIRenderer::Initialize(const NativeWindowHandle* nativeHandle)
{
    Renderer::Initialize(nativeHandle);

    ImGui_ImplMetal_Init(hs_rhi_to_layer(_layer).device);
    ImGui_ImplSDL3_InitForMetal(static_cast<SDL_Window*>(nativeHandle->window));
    
//    _renderPassDescriptor = [MTLRenderPassDescriptor new];
}

void GUIRenderer::NextFrame(Swapchain* swapchain)
{
    _commandBuffer = nil;
    _commandEncoder = nil;
    
    Renderer::NextFrame(swapchain);
    
    RenderPass* renderPass = swapchain->GetRenderPass();
    MTLRenderPassDescriptor* rpDesc = (__bridge MTLRenderPassDescriptor*)renderPass->handle;
    ImGui_ImplMetal_NewFrame(rpDesc);
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}


void GUIRenderer::RenderGUI()
{
    _commandBuffer = (__bridge id<MTLCommandBuffer>)_curCommandBuffer;
    _commandEncoder = (__bridge id<MTLRenderCommandEncoder>) _curCommandEncoder;
    
    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), _commandBuffer, _commandEncoder);

    ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    
    [_commandEncoder endEncoding];
}
//
//void GUIRenderer::Present(Swapchain* swapchain)
//{
//    id<CAMetalDrawable> currentDrawable = (__bridge id<CAMetalDrawable>)swapchain->GetDrawable();
//    
//    [_commandBuffer presentDrawable:currentDrawable];
//
//    [_commandBuffer commit];
//}

void GUIRenderer::Shutdown()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    Renderer::Shutdown();
}

HS_NS_EDITOR_END
