//
//  Renderer.mm
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//
#include "Engine/Renderer/Renderer.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Window.h"
#include "Engine/Core/EngineContext.h"

#include "Engine/Core/Swapchain.h"
#include "Engine/RendererPass/RendererPass.h"

 HS_NS_BEGIN
//
// namespace
//{
// id<MTLDevice>       s_device;
// id<MTLCommandQueue> s_commandQueue;
// CAMetalLayer*       s_layer;
//
//// TODO: 테스트용 임시
////    id<MTLDevice> _device;
////    id<MTLCommandQueue> _commandQueue;
//
// id<MTLRenderCommandEncoder> _renderEncoder;
//
// id<MTLTexture>           _renderTarget[Renderer::MAX_SUBMIT_INDEX];
// id<CAMetalDrawable>      _currentDrawable;
// MTLRenderPassDescriptor* _currentRpDesc;
// RenderPass* _currentRp;
//
//} // namespace

Renderer::Renderer(RHIContext* context)
    : _rhiContext(context)
    , _swapchain(nullptr)
    , _currentRenderTarget(nullptr)
    , _frameIndex(0)
{
}

Renderer::~Renderer()
{
    Shutdown();
}

bool Renderer::Initialize()
{
    for (int i = 0; i < 3; i++)
    {
        _commandBuffer[i] = _rhiContext->CreateCommandBuffer();
    }
    _isInitialized = true;

    return _isInitialized;
}

// void Renderer::NextFrame(Swapchain* swapchain)
//{
//     swapchain->_frameCount = (swapchain->_frameCount + 1) % MAX_FRAME_COUNT;
//
//     int width, height;
//
//     SDL_Window* window = static_cast<SDL_Window*>(_window);
//     SDL_GetWindowSizeInPixels(window, &width, &height);
//     hs_rhi_to_layer(_layer).drawableSize = CGSizeMake(width, height);
//     _currentDrawable                     = [hs_rhi_to_layer(_layer) nextDrawable];
//     swapchain->SetDrawable((__bridge void*)_currentDrawable);
//     swapchain->Update(width, height);
//
//     RenderPass* rp = swapchain->GetRenderPass();
//
//     MTLRenderPassDescriptor* rpDesc = (__bridge MTLRenderPassDescriptor*)rp->handle;
//
//     rpDesc.colorAttachments[0].clearColor  = MTLClearColorMake(0.2f, 0.2f, 0.2f, 1.0f);
//     rpDesc.colorAttachments[0].texture     = _currentDrawable.texture;
//     rpDesc.colorAttachments[0].loadAction  = MTLLoadActionClear;
//     rpDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
//
//     rp->info.colorAttachments[0].loadAction  = hs_rhi_from_load_action(rpDesc.colorAttachments[0].loadAction);
//     rp->info.colorAttachments[0].storeAction = hs_rhi_from_store_action(rpDesc.colorAttachments[0].storeAction);
//
//     _currentRp = rp;
//     _currentRpDesc = rpDesc;
// }
//
// void Renderer::Render(const RenderParameter& param, Framebuffer* renderTarget)
//{
//     id<MTLCommandBuffer> commandBuffer    = [s_commandQueue commandBuffer];
//     _curCommandBuffer = (__bridge_retained void*)commandBuffer;
//
//     for (auto* pass : _rendererPasses)
//     {
//         pass->OnBeforeRendering(_frameCount);
//     }
//
//     id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:_currentRpDesc];
//     _curCommandEncoder                  = (__bridge_retained void*)encoder;
//
//
//     for (auto* pass : _rendererPasses)
//     {
//         pass->Configure(renderTarget);
//
//         pass->Execute(_curCommandEncoder, _currentRp);
//     }
//
//     for (auto* pass : _rendererPasses)
//     {
//         pass->OnAfterRendering();
//     }
// }

void Renderer::Shutdown()
{
    for (int i = 0; i < 3; i++)
    {
        if (nullptr != _commandBuffer[i])
        {
            delete _commandBuffer[i];
            _commandBuffer[i] = nullptr;
        }
    }

    for (size_t i = 0; i < _rendererPasses.size(); i++)
    {
        delete _rendererPasses[i];
        _rendererPasses[i] = nullptr;
    }
    _rendererPasses.clear();

    _isInitialized = false;
}

 HS_NS_END
