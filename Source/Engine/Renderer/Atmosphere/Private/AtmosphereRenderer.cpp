//
//  AtmosphereRenderer.cpp
//  Engine/Renderer/Atmosphere
//

#include "Engine/Renderer/Atmosphere/AtmosphereRenderer.h"
#include "Engine/Renderer/Atmosphere/AtmospherePrecompute.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"

HS_NS_BEGIN

AtmosphereRenderer::AtmosphereRenderer()
{
}

AtmosphereRenderer::~AtmosphereRenderer()
{
	Finalize();
}

bool AtmosphereRenderer::Initialize(RHIContext* context, const AtmosphereParameters& params,
                                     int numScatteringOrders)
{
	if (_precompute != nullptr)
	{
		HS_LOG(warning, "AtmosphereRenderer already initialized");
		return false;
	}

	_context = context;

	// Create precompute system
	_precompute = new AtmospherePrecompute();
	if (!_precompute->Initialize(context, params, numScatteringOrders))
	{
		HS_LOG(error, "Failed to initialize AtmospherePrecompute");
		delete _precompute;
		_precompute = nullptr;
		return false;
	}

	// Create sampler for LUT texture sampling (linear filtering, clamp)
	SamplerInfo samplerInfo{};
	samplerInfo.type = ETextureType::TEX_2D;
	samplerInfo.minFilter = EFilterMode::LINEAR;
	samplerInfo.magFilter = EFilterMode::LINEAR;
	samplerInfo.mipmapMode = EFilterMode::LINEAR;
	samplerInfo.addressU = EAddressMode::CLAMP_TO_EDGE;
	samplerInfo.addressV = EAddressMode::CLAMP_TO_EDGE;
	samplerInfo.addressW = EAddressMode::CLAMP_TO_EDGE;
	samplerInfo.isPixelCoordinate = false;

	_lutSampler = context->CreateSampler("Atmosphere LUT Sampler", samplerInfo);
	if (_lutSampler == nullptr)
	{
		HS_LOG(error, "Failed to create LUT sampler");
		Finalize();
		return false;
	}

	return true;
}

void AtmosphereRenderer::Finalize()
{
	if (_lutSampler != nullptr)
	{
		_context->DestroySampler(_lutSampler);
		_lutSampler = nullptr;
	}

	if (_precompute != nullptr)
	{
		_precompute->Finalize();
		delete _precompute;
		_precompute = nullptr;
	}

	_context = nullptr;
}

void AtmosphereRenderer::Precompute(RHICommandBuffer* cmdBuffer)
{
	if (_precompute == nullptr)
	{
		HS_LOG(error, "AtmosphereRenderer not initialized");
		return;
	}

	_precompute->Compute(cmdBuffer);
}

bool AtmosphereRenderer::IsPrecomputed() const
{
	return _precompute != nullptr && _precompute->IsComputed();
}

RHITexture* AtmosphereRenderer::GetTransmittanceTexture() const
{
	return _precompute ? _precompute->GetTransmittanceTexture() : nullptr;
}

RHITexture* AtmosphereRenderer::GetScatteringTexture() const
{
	return _precompute ? _precompute->GetScatteringTexture() : nullptr;
}

RHITexture* AtmosphereRenderer::GetIrradianceTexture() const
{
	return _precompute ? _precompute->GetIrradianceTexture() : nullptr;
}

RHITexture* AtmosphereRenderer::GetSingleMieScatteringTexture() const
{
	return _precompute ? _precompute->GetSingleMieScatteringTexture() : nullptr;
}

const AtmosphereParameters& AtmosphereRenderer::GetParameters() const
{
	static AtmosphereParameters defaultParams;
	return _precompute ? _precompute->GetParameters() : defaultParams;
}

HS_NS_END
