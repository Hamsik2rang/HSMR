//
//  Renderer.cpp
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//
#include "Renderer/Renderer.h"

#include "Core/Log.h"
#include "Core/SystemContext.h"

#include "RHI/Swapchain.h"
#include "Renderer/RenderDefinition.h"
#include "Renderer/RenderPass/RenderPass.h"
#include "Renderer/RenderResourceManager.h"
#include "Renderer/ShaderLibrary.h"

HS_NS_BEGIN

Renderer::Renderer(RHIContext* context)
    : _rhiContext(context)
    , _currentRenderTarget(nullptr)
    , frameIndex(0)
{
}

Renderer::~Renderer()
{
    Shutdown();
}

bool Renderer::Initialize()
{
    _resourceManager = new RenderResourceManager(_rhiContext);

    _shaderLibrary = new ShaderLibrary();
    std::string shaderSourceDir = SystemContext::Get()->assetDirectory + "Shaders";
    if (!_shaderLibrary->Initialize(shaderSourceDir))
    {
        HS_LOG(error, "[Renderer] ShaderLibrary initialization failed");
    }

    _graphBuilder.Initialize(_rhiContext);

    _isInitialized = true;

    return _isInitialized;
}

void Renderer::NextFrame(Swapchain* swapchain)
{
    frameIndex = _rhiContext->AcquireNextImage(swapchain);
    if (frameIndex == UINT32_MAX)
    {
        _rhiContext->Restore(swapchain);
        return;
    }
    _curCommandBuffer = swapchain->GetCommandBufferForCurrentFrame();
}

void Renderer::Render(
    Scene* scene,
    RenderTarget* renderTarget)
{
    Render(scene, renderTarget, RenderOptions{});
}

void Renderer::Render(
    Scene* scene,
    RenderTarget* renderTarget,
    const RenderOptions& options)
{
    (void)options;
    SceneResource sceneResource = _resourceManager->BuildSceneResource(scene, _shaderLibrary);

    for (auto* pass : _rendererPasses)
    {
        pass->OnBeforeRendering(frameIndex);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->Configure(renderTarget);

        pass->Execute(_curCommandBuffer, sceneResource);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->OnAfterRendering();
    }
}

void Renderer::Render(
    const RenderSceneSnapshot& snapshot,
    RenderTarget* renderTarget)
{
    Render(snapshot, renderTarget, RenderOptions{});
}

void Renderer::Render(
    const RenderSceneSnapshot& snapshot,
    RenderTarget* renderTarget,
    const RenderOptions& options)
{
    (void)options;
    SceneResource sceneResource = _resourceManager->BuildSceneResource(snapshot);

    for (auto* pass : _rendererPasses)
    {
        pass->OnBeforeRendering(frameIndex);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->Configure(renderTarget);

        pass->Execute(_curCommandBuffer, sceneResource);
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

    if (_shaderLibrary)
    {
        _shaderLibrary->Shutdown();
        delete _shaderLibrary;
        _shaderLibrary = nullptr;
    }

    if (_resourceManager)
    {
        delete _resourceManager;
        _resourceManager = nullptr;
    }

    _isInitialized = false;
}

HS_NS_END
