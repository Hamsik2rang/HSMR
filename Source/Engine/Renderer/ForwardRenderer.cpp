#include "Engine/Renderer/ForwardRenderer.h"

HS_NS_BEGIN

ForwardRenderer::ForwardRenderer(RHIContext* rhiContext)
    : Renderer(rhiContext)
{
}

ForwardRenderer::~ForwardRenderer()
{
}

RenderTargetInfo ForwardRenderer::GetBareboneRenderTargetInfo()
{
    static TextureInfo colorTextureInfo{};
    colorTextureInfo.arrayLength = 1;
	colorTextureInfo.extent.width = 1;
	colorTextureInfo.extent.height = 1;
	colorTextureInfo.extent.depth = 1;
	colorTextureInfo.format = EPixelFormat::R8G8B8A8_SRGB;
	colorTextureInfo.isCompressed = false;
	colorTextureInfo.isDepthStencilBuffer = false;
	colorTextureInfo.mipLevel = 1;
	colorTextureInfo.type = ETextureType::TEX_2D;
	colorTextureInfo.usage = ETextureUsage::COLOR_ATTACHMENT | ETextureUsage::INPUT_ATTACHMENT;
	colorTextureInfo.byteSize = 4 * colorTextureInfo.extent.width * colorTextureInfo.extent.height * colorTextureInfo.extent.depth;
	//...

    static RenderTargetInfo info{};
    info.width = 1;
    info.height = 1;
    info.colorTextureCount = 1;
	info.colorTextureInfos = { colorTextureInfo };
	//...

    return info;
}

HS_NS_END
