#include "Renderer/RenderTarget.h"

#include "RHI/RHIContext.h"
#include "RHI/RHIDefinition.h"
#include "RHI/Swapchain.h"

#include "Core/Log.h"

HS_NS_BEGIN

RenderTarget::RenderTarget(const RenderTargetInfo& info)
    : _info(info)
{
    Create(_info);
}

RenderTarget::~RenderTarget()
{
    Clear();
}

void RenderTarget::Create(const RenderTargetInfo& info)
{
    HS_CHECK(info.colorTextureCount >= 1, "Count of ColorTexture should be at least 1 or more");

    RHIContext* rhiContext = RHIContext::Get();

    if (true == info.isSwapchainTarget)
    {
        HS_CHECK(info.colorTextureCount == 1, "Swapchain RenderTarget must have only 1 ColorTexture");
        HS_CHECK(info.swapchain != nullptr, "Swapchain RenderTarget requires a non-null swapchain");

        // The color texture is owned by the swapchain — GetColorTexture(0) delegates to it.
        // Do not allocate here; the swapchain rotates its drawable handle every frame.
        _swapchain = info.swapchain;
    }
    else
    {
        for (size_t i = 0; i < info.colorTextureCount; i++)
        {
            RHITexture* texture = rhiContext->CreateTexture("RenderTarget Color Texture", nullptr, info.colorTextureInfos[i]);
            _colorTextures.push_back(texture);
        }
    }

    if (info.useDepthStencilTexture)
    {
        _depthStencilTexture = rhiContext->CreateTexture("RenderTarget DepthStencil Teture", nullptr, info.depthStencilInfo);
    }

    //... Resolve Target

    _info = info;
}

RHITexture* RenderTarget::GetColorTexture(uint32 index) const
{
    if (_swapchain != nullptr && index == 0)
    {
        return _swapchain->GetCurrentColorTexture();
    }
    return _colorTextures[index];
}

void RenderTarget::Update(const RenderTargetInfo& info)
{
    Clear();
    
    Create(info);
}

void RenderTarget::Update(uint32 width, uint32 height)
{
    if (width == 0 || height == 0)
    {
        HS_ASSERT(false, "RenderTarget::Update() called with zero dimension (width=%u, height=%u)", width, height);
        return;
    }

    if (_info.width == width && _info.height == height)
    {
        return;
    }

    RenderTargetInfo updateInfo = _info;
    updateInfo.width = width;
    updateInfo.height = height;
    for (size_t i = 0; i < updateInfo.colorTextureCount; i++)
    {
        updateInfo.colorTextureInfos[i].extent.width  = width;
        updateInfo.colorTextureInfos[i].extent.height = height;
    }

    if (updateInfo.useDepthStencilTexture)
    {
        updateInfo.depthStencilInfo.extent.width  = width;
        updateInfo.depthStencilInfo.extent.height = height;
    }

    Clear();

    Create(updateInfo);
}

void RenderTarget::Clear()
{
    RHIContext* rc = RHIContext::Get();
    rc->WaitForIdle();

    // Skip color destruction for swapchain-backed RTs — the swapchain owns those textures.
    if (false == _info.isSwapchainTarget)
    {
        for (size_t i = 0; i < _colorTextures.size(); i++)
        {
            if (nullptr == _colorTextures[i])
            {
                continue;
            }

            rc->DestroyTexture(_colorTextures[i]);
        }
    }
    _colorTextures.clear();

    if (_info.useDepthStencilTexture && nullptr != _depthStencilTexture)
    {
        rc->DestroyTexture(_depthStencilTexture);
    }
    _depthStencilTexture = nullptr;

    _swapchain = nullptr;
    _info      = {};
}

HS_NS_END
