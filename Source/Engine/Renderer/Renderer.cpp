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

#include "Engine/RHI/Swapchain.h"
#include "Engine/RendererPass/RendererPass.h"

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

RenderPass* Renderer::RHIHandleCache::GetRenderPass(const RenderPassInfo& info)
{
    uint32 hash = Hasher<RenderPassInfo>::Get(info);

    if (_renderPassCache.find(hash) == _renderPassCache.end())
    {
        RenderPass* renderPass = _renderer->GetRHIContext()->CreateRenderPass(info);

        _renderPassCache.insert(std::make_pair(hash, renderPass));
    }

    return _renderPassCache[hash];
}

Framebuffer* Renderer::RHIHandleCache::GetFramebuffer(RenderPass* renderPass, RenderTarget* renderTarget)
{
    uint32 hash = HashCombine(Hasher<RenderTarget>::Get(*renderTarget), Hasher<RenderPass>::Get(*renderPass));

    if (_framebufferCache.find(hash) == _framebufferCache.end())
    {
        FramebufferInfo fbInfo{};
        fbInfo.width                  = renderTarget->GetWidth();
        fbInfo.height                 = renderTarget->GetHeight();
        fbInfo.colorBuffers           = renderTarget->GetColorTextures(); // COPY?!
        fbInfo.depthStencilBuffer     = renderTarget->GetDepthStencilTexture();
        fbInfo.isSwapchainFramebuffer = renderPass->info.isSwapchainRenderPass;
        fbInfo.renderPass             = renderPass;

        Framebuffer* fb = _renderer->GetRHIContext()->CreateFramebuffer(fbInfo);

        _framebufferCache.insert(std::make_pair(hash, fb));
    }

    return _framebufferCache[hash];
}

GraphicsPipeline* Renderer::RHIHandleCache::GetGraphicsPipeline(const GraphicsPipelineInfo& info)
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
    _isInitialized = true;

    return _isInitialized;
}

void Renderer::NextFrame(Swapchain* swapchain)
{
    frameIndex = _rhiContext->AcquireNextImage(swapchain);
    if (frameIndex == UINT32_MAX)
    {
        // Swapchain needs to be recreated
        HS_LOG(warning, "Failed to acquire next image, swapchain may need recreation");
        // TODO: Handle swapchain recreation
        return;
    }
    _curCommandBuffer = swapchain->GetCommandBufferForCurrentFrame();
}

void Renderer::Render(const RenderParameter& param, RenderTarget* renderTarget)
{
    for (auto* pass : _rendererPasses)
    {
        pass->OnBeforeRendering(frameIndex);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->Configure(renderTarget);

        RenderPass* renderPass = GetHandleCache()->GetRenderPass(pass->GetFixedSettingForCurrentPass());

        pass->Execute(_curCommandBuffer, renderPass);
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
