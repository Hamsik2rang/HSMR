//
//  RenderPath.mm
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//
#include "Renderer/RenderPath.h"

#include "Core/Log.h"

#include "RHI/Swapchain.h"
#include "Renderer/RenderPass/RenderPass.h"

HS_NS_BEGIN

RenderPath::RHIHandleCache::RHIHandleCache(RenderPath* renderer)
    : _renderer(renderer)
    , _renderPassCache()
    , _framebufferCache()
    , _gPipelineCache()
{
}

RenderPath::RHIHandleCache::~RHIHandleCache()
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

RHIRenderPass* RenderPath::RHIHandleCache::GetRenderPass(const RenderPassInfo& info)
{
    uint32 hash = Hasher<RenderPassInfo>::Get(info);

    if (_renderPassCache.find(hash) == _renderPassCache.end())
    {
        RHIRenderPass* renderPass = _renderer->GetRHIContext()->CreateRenderPass("RenderPass", info);

        _renderPassCache.insert(std::make_pair(hash, renderPass));
    }

    return _renderPassCache[hash];
}

RHIFramebuffer* RenderPath::RHIHandleCache::GetFramebuffer(RHIRenderPass* renderPass, RenderTarget* renderTarget)
{
    uint32 hash = HashCombine(Hasher<RenderTarget>::Get(*renderTarget), Hasher<RHIRenderPass>::Get(*renderPass));

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

RHIGraphicsPipeline* RenderPath::RHIHandleCache::GetGraphicsPipeline(const GraphicsPipelineInfo& info)
{
    return nullptr;
}

RenderPath::RenderPath(RHIContext* context)
    : _rhiContext(context)
    , _currentRenderTarget(nullptr)
    , frameIndex(0)
    , _rhiHandleCache(nullptr)
{
}

RenderPath::~RenderPath()
{
    Shutdown();
}

bool RenderPath::Initialize()
{
    _rhiHandleCache = new RHIHandleCache(this);
    _isInitialized = true;

    return _isInitialized;
}

void RenderPath::NextFrame(Swapchain* swapchain)
{
    frameIndex = _rhiContext->AcquireNextImage(swapchain);
    if (frameIndex == UINT32_MAX)
    {
        _rhiContext->Restore(swapchain);
        return;
    }
    _curCommandBuffer = swapchain->GetCommandBufferForCurrentFrame();
}

void RenderPath::Render(const RenderParameter& param, RenderTarget* renderTarget)
{
    for (auto* pass : _rendererPasses)
    {
        pass->OnBeforeRendering(frameIndex);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->Configure(renderTarget);

        RHIRenderPass* renderPass = GetHandleCache()->GetRenderPass(pass->GetFixedSettingForCurrentPass());

        pass->Execute(_curCommandBuffer, renderPass);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->OnAfterRendering();
    }
}

void RenderPath::Shutdown()
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
