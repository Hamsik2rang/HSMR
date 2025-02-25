#include "Engine/Renderer/ForwardRenderer.h"

HS_NS_BEGIN

const RenderTargetInfo ForwardRenderer::s_bareboneRenderTargetInfo = {
    1, 1, 1, std::vector<TextureInfo>(1, TextureInfo{EPixelFormat::R8G8B8A8_SRGB, ETextureType::TEX_2D, ETextureUsage::RENDER_TARGET | ETextureUsage::SHADER_READ, {}, 1, 1, 0, false, false, false, false}), true, {EPixelFormat::DEPTH32, ETextureType::TEX_2D, ETextureUsage::RENDER_TARGET | ETextureUsage::SHADER_READ, {}, 1, 1, 0, false, false, true, false}, false
};

ForwardRenderer::ForwardRenderer(RHIContext* rhiContext)
    : Renderer(rhiContext)
{
}

ForwardRenderer::~ForwardRenderer()
{
}

HS_NS_END
