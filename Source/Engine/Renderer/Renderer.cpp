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
    _isInitialized = true;

    return _isInitialized;
}

void Renderer::NextFrame(Swapchain* swapchain)
{
    _frameIndex       = _rhiContext->AcquireNextImage(swapchain);
    _curCommandBuffer = swapchain->GetCommandBufferForCurrentFrame();
}

void Renderer::Render(const RenderParameter& param, RenderTarget* renderTarget)
{
    for (auto* pass : _rendererPasses)
    {
        pass->OnBeforeRendering(_frameIndex);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->Configure(renderTarget);

        pass->Execute(_curCommandBuffer, nullptr);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->OnAfterRendering();
    }
}

void Renderer::Shutdown()
{
    for (size_t i = 0; i < _rendererPasses.size(); i++)
    {
        if (nullptr != _rendererPasses[i])
        {
            delete _rendererPasses[i];
            _rendererPasses[i] = nullptr;
        }
    }
    _rendererPasses.clear();
    _curCommandBuffer = nullptr;

    _isInitialized = false;
}

HS_NS_END
