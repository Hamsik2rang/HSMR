#include "Engine/Renderer/RenderTarget.h"

#include "Engine/RHI/RHIDefinition.h"
#include "Engine/Core/EngineContext.h"

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

    RHIContext* rhiContext = hs_engine_get_context()->rhiContext;

    if (true == info.isSwapchainTarget)
    {
        HS_CHECK(info.colorTextureCount == 1, "Swapchain RenderTarget must have only 1 ColorTexture");

        _colorTextures.push_back(rhiContext->CreateTexture(nullptr, info.colorTextureInfos[0]));
    }
    else
    {
//        _colorTextures.resize(info.colorTextureCount);

        for (size_t i = 0; i < info.colorTextureCount; i++)
        {
            
            Texture* texture = rhiContext->CreateTexture(nullptr, info.colorTextureInfos[i]);
            _colorTextures.push_back(texture);
        }
    }

    if (info.useDepthStencilTexture)
    {
        _depthStencilTexture = rhiContext->CreateTexture(nullptr, info.depthStencilInfo);
    }

    //... Resolve Target
    
    _info = info;
}

void RenderTarget::Update(const RenderTargetInfo& info)
{
    Clear();
    
    Create(info);
}

void RenderTarget::Update(uint32 width, uint32 height)
{
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
    RHIContext* rc = hs_engine_get_context()->rhiContext;
    rc->WaitForIdle();
    for (size_t i = 0; i < _info.colorTextureCount; i++)
    {
        if (nullptr == _colorTextures[i])
        {
            continue;
        }

        rc->DestroyTexture(_colorTextures[i]);
    }
    _colorTextures.clear();

    if (_info.useDepthStencilTexture && nullptr != _depthStencilTexture)
    {
        rc->DestroyTexture(_depthStencilTexture);
    }
    _depthStencilTexture = nullptr;

    //...
    _info   = {};
}

HS_NS_END
