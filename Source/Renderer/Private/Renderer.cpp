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

Renderer::RHIHandleCache::RHIHandleCache(Renderer* renderer)
    : _renderer(renderer)
    , _renderPassCache()
    , _framebufferCache()
    , _gPipelineCache()
{
}

Renderer::RHIHandleCache::~RHIHandleCache()
{
    //...
    RHIContext* rhiContext = _renderer->GetRHIContext();
    rhiContext->WaitForIdle();

    for (auto& elem : _renderPassCache)
    {
        if (nullptr != elem.second)
        {
            rhiContext->DestroyRenderPass(elem.second);
            elem.second = nullptr;
        }
    }
    _renderPassCache.clear();

    for (auto& elem : _framebufferCache)
    {
        if (nullptr != elem.second)
        {
            rhiContext->DestroyFramebuffer(elem.second);
            elem.second = nullptr;
        }
    }
    _framebufferCache.clear();

    for (auto& elem : _gPipelineCache)
    {
        if (nullptr != elem.second)
        {
            rhiContext->DestroyGraphicsPipeline(elem.second);
            elem.second = nullptr;
        }
    }
    _gPipelineCache.clear();
}

RHIRenderPass* Renderer::RHIHandleCache::GetRenderPass(const RenderPassInfo& info)
{
    size_t hash = std::hash<RenderPassInfo>{}(info);

    if (_renderPassCache.find(hash) == _renderPassCache.end())
    {
        RHIRenderPass* renderPass = _renderer->GetRHIContext()->CreateRenderPass("RenderPass", info);

        _renderPassCache.insert(std::make_pair(hash, renderPass));
    }

    return _renderPassCache[hash];
}

RHIFramebuffer* Renderer::RHIHandleCache::GetFramebuffer(RHIRenderPass* renderPass, RenderTarget* renderTarget)
{
    size_t hash = HashCombine64(std::hash<RenderTarget>{}(*renderTarget), std::hash<RHIRenderPass>{}(*renderPass));

    if (_framebufferCache.find(hash) == _framebufferCache.end())
    {
        FramebufferInfo fbInfo{};
        fbInfo.width                  = renderTarget->GetWidth();
        fbInfo.height                 = renderTarget->GetHeight();
        fbInfo.colorBuffers           = renderTarget->GetColorTextures(); // COPY?!
        fbInfo.depthStencilBuffer     = renderTarget->GetDepthStencilTexture();
        fbInfo.isSwapchainFramebuffer = renderPass->info.isSwapchainRenderPass;
        fbInfo.renderPass             = renderPass;

        RHIFramebuffer* fb = _renderer->GetRHIContext()->CreateFramebuffer("Framebuffer", fbInfo);

        _framebufferCache.insert(std::make_pair(hash, fb));
    }

    return _framebufferCache[hash];
}

RHIGraphicsPipeline* Renderer::RHIHandleCache::GetGraphicsPipeline(const GraphicsPipelineInfo& info)
{
    return nullptr;
}

Renderer::Renderer(RHIContext* context)
    : _rhiContext(context)
    , _currentRenderTarget(nullptr)
    , frameIndex(0)
    , _rhiHandleCache(nullptr)
{
}

Renderer::~Renderer()
{
    Shutdown();
}

bool Renderer::Initialize()
{
    _rhiHandleCache = new RHIHandleCache(this);

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
    SceneResource sceneResource = _resourceManager->BuildSceneResource(scene, _shaderLibrary);

    for (auto* pass : _rendererPasses)
    {
        pass->OnBeforeRendering(frameIndex);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->Configure(renderTarget);

        RHIRenderPass* renderPass = GetHandleCache()->GetRenderPass(pass->GetFixedSettingForCurrentPass());

        pass->Execute(_curCommandBuffer, renderPass, sceneResource);
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
    SceneResource sceneResource = _resourceManager->BuildSceneResource(snapshot);

    for (auto* pass : _rendererPasses)
    {
        pass->OnBeforeRendering(frameIndex);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->Configure(renderTarget);

        RHIRenderPass* renderPass = GetHandleCache()->GetRenderPass(pass->GetFixedSettingForCurrentPass());

        pass->Execute(_curCommandBuffer, renderPass, sceneResource);
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
