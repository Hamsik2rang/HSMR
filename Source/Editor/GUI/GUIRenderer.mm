#include "Editor/GUI/GUIRenderer.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Renderer/RenderDefinition.h"

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
id<MTLRenderCommandEncoder> _renderEncoder;

id<MTLTexture>           _renderTarget[Renderer::MAX_SUBMIT_INDEX];
id<CAMetalDrawable>      _currentDrawable;
MTLRenderPassDescriptor* _renderPassDescriptor;
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

    ImGui_ImplMetal_Init(METAL_LAYER(_layer).device);
    ImGui_ImplSDL3_InitForMetal(static_cast<SDL_Window*>(nativeHandle->window));
    
    _renderPassDescriptor = [MTLRenderPassDescriptor new];
}

void GUIRenderer::NextFrame()
{
    _submitIndex = (_submitIndex + 1) % MAX_SUBMIT_INDEX;

    int width, height;
    
    SDL_Window* window = static_cast<SDL_Window*>(_window);
    SDL_GetWindowSizeInPixels(window, &width, &height);
    METAL_LAYER(_layer).drawableSize = CGSizeMake(width, height);
    _currentDrawable                 = [METAL_LAYER(_layer) nextDrawable];
}

void GUIRenderer::Render(const RenderParameter& param, RenderTexture* renderTarget)
{
    _commandBuffer = [METAL_COMMAND_QUEUE(_commandQueue) commandBuffer];

    _renderPassDescriptor.colorAttachments[0].clearColor  = MTLClearColorMake(0.2f, 0.2f, 0.2f, 1.0f);
    _renderPassDescriptor.colorAttachments[0].texture     = _currentDrawable.texture;
    _renderPassDescriptor.colorAttachments[0].loadAction  = MTLLoadActionClear;
    _renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    _renderEncoder = [_commandBuffer renderCommandEncoderWithDescriptor:_renderPassDescriptor];

    ImGui_ImplMetal_NewFrame(_renderPassDescriptor);
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    
    // ... GUI
    ImGui::ShowDemoWindow();

    //    Renderer::renderDockingPanel();

    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), _commandBuffer, _renderEncoder);

    ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    [_renderEncoder endEncoding];
}

void GUIRenderer::Present()
{
    [_commandBuffer presentDrawable:_currentDrawable];

    [_commandBuffer commit];
}

void GUIRenderer::Shutdown()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    Renderer::Shutdown();
}

HS_NS_EDITOR_END
