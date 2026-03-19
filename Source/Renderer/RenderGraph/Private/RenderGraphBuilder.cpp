#include "Renderer/RenderGraph/RenderGraphBuilder.h"

HS_NS_BEGIN

RenderGraphBuilder::RenderGraphBuilder()
{}

RenderGraphBuilder::~RenderGraphBuilder()
{}

RGTexture* RenderGraphBuilder::RegisterExternalTexture(RHITexture* texture)
{
	if (_externalRHIHandleMap.find(texture) != _externalRHIHandleMap.end())
	{
		return static_cast<RGTexture*>(_externalRHIHandleMap[texture]);
	}

	RGTextureDescriptor desc =
	{
		.info = texture->info,
		.access = ERGTextureAccess::ReadOnly,
		.name = texture->GetName()
	};
	_externalRHIHandleMap[texture] = _allocator.Allocate<RGTexture>(_frameIndex, desc);
	RGTexture* rgTexture = static_cast<RGTexture*>(_externalRHIHandleMap[texture]);
	rgTexture->_rhiTexture = texture;

	return rgTexture;
}

RGTexture* RenderGraphBuilder::CreateTexture(const RGTextureDescriptor& desc)
{
	RGTexture* texture = _allocator.Allocate<RGTexture>(_frameIndex, desc);

	return texture;
}

RGTexture* RenderGraphBuilder::FindTexture(RHITexture* texture) const
{
	auto it = _externalRHIHandleMap.find(texture);
	if (it != _externalRHIHandleMap.end())
	{
		return static_cast<RGTexture*>(it->second);
	}
	return nullptr;
}

RGTexture* RenderGraphBuilder::FindTexture(uint32 id) const
{
	// TODO: 구현 필요
	return nullptr;
}

RGBuffer* RenderGraphBuilder::RegisterExternalBuffer(RHIBuffer* buffer)
{
	if (_externalRHIHandleMap.find(buffer) != _externalRHIHandleMap.end())
	{
		return static_cast<RGBuffer*>(_externalRHIHandleMap[buffer]);
	}

	RGBufferDescriptor desc
	{
		.info = buffer->info,
		.access = ERGBufferAccess::ReadOnly,
		.name = buffer->GetName()
	};
	_externalRHIHandleMap[buffer] = _allocator.Allocate<RGBuffer>(_frameIndex, desc);
	RGBuffer* rgBuffer = static_cast<RGBuffer*>(_externalRHIHandleMap[buffer]);
	rgBuffer->_rhiBuffer = buffer;

	return rgBuffer;
}

RGBuffer* RenderGraphBuilder::CreateBuffer(const RGBufferDescriptor& desc)
{
	RGBuffer* buffer = _allocator.Allocate<RGBuffer>(_frameIndex, desc);

	return buffer;
}

RGBuffer* RenderGraphBuilder::FindBuffer(RHIBuffer* buffer) const
{
	// TODO: 구현 필요
	return nullptr;
}

void RenderGraphBuilder::Setup(RHICommandBuffer* cmdBuffer)
{
	_currentCmdBuffer = cmdBuffer;
	_frameIndex = (_frameIndex + 1) % s_maxFramesInFlight;
}

void RenderGraphBuilder::Compile()
{
	size_t executablePassCount = 0;
	// 1. 각 패스별로 의존성 분석하기 (및 Culling 여부 결정하기)
	for (auto& pass : _passes)
	{
		HS_ASSERT(pass->IsCompiled() == false, "Pass %s is already compiled!", pass->_name);

		if (_resourceDependencyMap.find(pass) == _resourceDependencyMap.end())
		{
			if (ERGPassFlag::None == (pass->GetFlags() & ERGPassFlag::NeverCull))
			{
				pass->_isCulled = true;
				continue;
			}
		}
		else
		{
			// TODO: RGPassParameters를 기반으로 의존성 분석하기
			// Parameters안의 Descriptor들을 기반으로 RDGResource를 만들고, RGPass와 RGResource를 연결하기
			// 이 때 RDGResource를 만들되 실제 RHIHandle과 연결은 하지 않고(null로 유지), RGPass의 upstream/downstream 관계만 설정하기
			// 근데 여기서 사용 리소스를 최소화하려면 어떤 체크들을 해야 할까?
			if (_resourceDependencyMap.find(pass) != _resourceDependencyMap.end())
			{
				for (auto* resource : _resourceDependencyMap[pass])
				{
					// Writers는 이 패스보다 먼저 실행되어야 하는 패스들, Readers는 이 패스보다 나중에 실행되어야 하는 패스들
					for (auto* writer : resource->_writers)
					{
						pass->_upstreams.push_back(writer);
						writer->_downstreams.push_back(pass);
					}
					for (auto* reader : resource->_readers)
					{
						pass->_downstreams.push_back(reader);
						reader->_upstreams.push_back(pass);
					}
				}
			}
		}
		executablePassCount++;
	}

	// 2. 위에서 분석된 의존성 정보를 기반으로 Topological Sort하기
	_executablePasses.reserve(executablePassCount);
	static std::vector<RGPass*> lastPass;
	if (!lastPass.empty())
	{
		lastPass.clear();
	}
	lastPass.reserve(executablePassCount);
	for (auto* pass : _passes)
	{
		RGPass* curPass = pass;
		// 1. 현재 패스의 Upstreams가 모두 Check되어 있거나 Culled되어 있으면 현재 패스를 Compiled로 표시하고 Sorted List에 추가
		// 2. 그렇지 않으면 Upstreams 중 하나를 선택해서 그 패스를 현재 패스로 설정하고 1로 돌아가기
		while ((false == curPass->_isChecked) && (false == curPass->IsCulled()))
		{
			bool allUpstreamsAreChecked = true;
			for (auto* up : curPass->_upstreams)
			{
				if (false == up->_isChecked)
				{
					allUpstreamsAreChecked = false;
					lastPass.push_back(curPass);
					curPass = up;
					break;
				}
			}

			if (allUpstreamsAreChecked)
			{
				curPass->_isChecked = true;
				_executablePasses.push_back(curPass);
				if (lastPass.empty())
				{
					// 복귀할 노드가 없으므로 break
					break;
				}

				curPass = lastPass.back();
				lastPass.pop_back();
			}
		}
	}
	HS_ASSERT(_executablePasses.size() == executablePassCount, "Topological Sort failed! Sorted Pass Count: %zu, Executable Pass Count: %zu", _executablePasses.size(), executablePassCount);

	// 3. Topological Sort된 패스를 기반으로 실제 RHIHandle과 RGResource를 연결하기
	// 모든 RGResource들의 [First Write, Last Read] 구간 파악 -> sortedPasses를 뒤에서부터 순회하면서 Read만날 떄 Pool에서 할당, Write만나면 Pool로 반환을 반복
	for (int i = static_cast<int>(executablePassCount) - 1; i >= 0; i--)
	{
		RGPass* curPass = _executablePasses[i];
		auto& dependentResources = _resourceDependencyMap[curPass];

		for (auto* resource : dependentResources)
		{
			RHIHandle* rhiHandle = resource->GetRHIHandle();
			if (nullptr != rhiHandle)
			{
				if (_externalRHIHandleMap.find(rhiHandle) != _externalRHIHandleMap.end())
				{
					// 외부에서 등록된 리소스라면 그냥 continue
					continue;
				}
				//else if()
				//{
				// TODO:
				//}
			}

			switch (resource->_type)
			{
			case RGResource::EType::Texture:
			{
				RGTexture* rgTexture = static_cast<RGTexture*>(resource);
				auto& freedTextureMap = _rhiTextureRegistry._freedTextures;
				auto& usedTextureMap = _rhiTextureRegistry._usedTextures;
				if (freedTextureMap.find(rgTexture->_desc.info) != freedTextureMap.end())
				{
					auto& freedPool = freedTextureMap[rgTexture->_desc.info];
					auto& usedPool = usedTextureMap[rgTexture->_desc.info];
					if (!freedPool.empty())
					{
						rgTexture->_rhiTexture = freedPool.back();
						freedPool.pop_back();
					}
					else
					{
						const char* name = rgTexture->_desc.name ? rgTexture->_desc.name : "RenderGraph Transient Texture";

						rgTexture->_rhiTexture = RHIContext::Get()->CreateTexture(name, nullptr, rgTexture->_desc.info);
						HS_ASSERT(rgTexture->_rhiTexture != nullptr, "Failed to create RHI Texture for RGTexture %s!", name);

						usedPool.push_back(rgTexture->_rhiTexture);
					}
				}
				break;
			}
			case RGResource::EType::Buffer:
			{
				break;
			}
			default:
				HS_ASSERT(false, "Unknown RGResource Type!");
			}
		}
	}
}

void RenderGraphBuilder::Execute()
{
	for (auto* pass : _executablePasses)
	{
		pass->Execute(*_currentCmdBuffer);
	}
}

void RenderGraphBuilder::Reset()
{
	_passes.clear();
	_allocator.Reset(_frameIndex);
	_currentCmdBuffer = nullptr;
}

void RenderGraphBuilder::AddDependency(RGResource* resource, RGPass* pass)
{
	if (_resourceDependencyMap.find(pass) == _resourceDependencyMap.end())
	{
		_resourceDependencyMap[pass] = std::vector<RGResource*>();
	}
	_resourceDependencyMap[pass].push_back(resource);
}

HS_NS_END