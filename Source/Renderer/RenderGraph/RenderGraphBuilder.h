#ifndef __HS_RENDER_GRAPH_BUILDER_H__
#define __HS_RENDER_GRAPH_BUILDER_H__

#include "Precompile.h"

#include "RHI/CommandHandle.h"
#include "RHI/RHIContext.h"
#include "Renderer/RenderDefinition.h"
#include "Renderer/RenderGraph/RenderGraphResource.h"

#include "Core/Memory/MemoryPool.h"

#include <functional>
#include <concepts>

HS_NS_BEGIN

class RenderGraphBuilder
{
public:
	RenderGraphBuilder();
	RenderGraphBuilder(const RenderGraphBuilder&) = delete;
	~RenderGraphBuilder();

	RenderGraphBuilder& operator=(const RenderGraphBuilder&) = delete;

	RGTexture* RegisterExternalTexture(RHITexture* texture);
	RGTexture* CreateTexture(const RGTextureDescriptor& desc);
	RGTexture* FindTexture(RHITexture* texture) const;
	RGTexture* FindTexture(uint32 id) const;

	RGBuffer* RegisterExternalBuffer(RHIBuffer* buffer);
	RGBuffer* CreateBuffer(const RGBufferDescriptor& desc);
	RGBuffer* FindBuffer(RHIBuffer* buffer) const;

	template <typename TPassParams,
		std::invocable<RenderGraphBuilder&, RGPass*, TPassParams*> TFnSetup,
		std::invocable<RHICommandBuffer&> TFnExecute>
	void AddPass(const char* passName,
		ERGPassFlag passFlags,
		TPassParams* passParams,
		TFnSetup fnSetup,
		TFnExecute fnExecute)
	{
		RGPass* pass = _allocator.Allocate<RGLambdaPass<TPassParams>>(_frameIndex, passName, passFlags, passParams, fnExecute);
		fnSetup(*this, pass, passParams);

		_passes.push_back(std::move(pass));
	}

	void Setup(RHICommandBuffer* cmdBuffer);
	void Compile();
	void Execute();
	void Reset();

	void AddDependency(RGResource* resource, RGPass* pass);

private:
	RHIContext* _rhiContext = nullptr;
	RHICommandBuffer* _currentCmdBuffer = nullptr;

	uint8 _frameIndex = static_cast<uint8>(-1);
	constexpr static uint8 s_maxFramesInFlight = 2;

	std::vector<Scoped<RHITexture>> _rhiTextures[s_maxFramesInFlight];
	std::vector<Scoped<RHIBuffer>> _rhiBuffers[s_maxFramesInFlight];

	std::vector<RGPass*> _passes;
	std::vector<RGPass*> _executablePasses;
	std::unordered_map<RGPass*, std::vector<RGResource*>> _resourceDependencyMap;
	std::unordered_map<RHIHandle*, RGResource*> _externalRHIHandleMap;

	struct RHITextureRegistry
	{
		std::unordered_map<TextureInfo, std::vector<RHITexture*>> _freedTextures;
		std::unordered_map<TextureInfo, std::vector<RHITexture*>> _usedTextures;
	} _rhiTextureRegistry;

	struct RHIBufferRgistry
	{
		std::unordered_map<BufferInfo, std::vector<RHIBuffer*>> _freedBuffers;
		std::unordered_map<BufferInfo, std::vector<RHIBuffer*>> _usedBuffers;
	} _rhiBufferRegistry;

	LinearAllocator<65536, 8, 2> _allocator;
};

HS_NS_END

#endif