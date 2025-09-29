#include "Renderer/RenderPass/Forward/ForwardOpaquePass.h"

#include "HAL/FileSystem.h"

#include "Renderer/RenderPath.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"

#include "Resource/Material.h"
#include "Resource/Mesh.h"

#include "Resource/ObjectManager.h"

HS_NS_BEGIN

ForwardOpaquePass::ForwardOpaquePass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder)
	: ForwardRenderPass(name, renderer, renderingOrder)
{
	createResourceHandles();
}

ForwardOpaquePass::~ForwardOpaquePass()
{}

void ForwardOpaquePass::OnBeforeRendering(uint32_t frameIndex)
{
	this->frameIndex = frameIndex;
}

void ForwardOpaquePass::Configure(RenderTarget* renderTarget)
{
	_currentRenderTarget = renderTarget;

	const RenderTargetInfo& rtInfo = _currentRenderTarget->GetInfo();

	_renderPassInfo.colorAttachmentCount = 1;
	Attachment ca{};
	ca.format = rtInfo.colorTextureInfos[0].format;
	ca.clearValue = ClearValue(0.0f, 0.0f, 0.0f, 1.0f);
	ca.isDepthStencil = false;
	ca.loadAction = ELoadAction::CLEAR;
	ca.storeAction = EStoreAction::STORE;
	_renderPassInfo.colorAttachments.push_back(ca);

	if (rtInfo.useDepthStencilTexture)
	{
		Attachment dsa{};
		dsa.format = rtInfo.depthStencilInfo.format;
		dsa.clearValue = ClearValue(1.0f, 0.0f);
		dsa.isDepthStencil = true;
		dsa.loadAction = ELoadAction::CLEAR;
		dsa.storeAction = EStoreAction::STORE;
		_renderPassInfo.depthStencilAttachment = dsa;
		_renderPassInfo.useDepthStencilAttachment = true;
	}

	_renderPassInfo.isSwapchainRenderPass = false;
}

void ForwardOpaquePass::Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass)
{
	if (nullptr == _gPipeline)
	{
		createPipelineHandles(renderPass);
	}

	RHIFramebuffer* framebuffer = _renderer->GetHandleCache()->GetFramebuffer(renderPass, _currentRenderTarget);

	float debugColor[4]{ 0.2f, 0.5f, 0.8f, 1.0f };

	commandBuffer->PushDebugMark("Opaque Pass", debugColor);
	Area area = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
	commandBuffer->BeginRenderPass(renderPass, framebuffer, area);

	commandBuffer->BindPipeline(_gPipeline);
	
	commandBuffer->SetViewport(Viewport{ 0.0f, 0.0f, static_cast<float>(framebuffer->info.width), static_cast<float>(framebuffer->info.height), 0.0f, 1.0f });
	
	commandBuffer->SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);

	uint32 offsets[2]{ 0, 0 };
	commandBuffer->BindVertexBuffers(&_vertexBuffer[0], offsets, 1);

	commandBuffer->DrawArrays(0, 6, 1);

	//commandBuffer->BindIndexBuffer(_indexBuffer);

	//commandBuffer->DrawIndexed(0, 36, 1, 0);

	commandBuffer->EndRenderPass();

	commandBuffer->PopDebugMark();

	//    id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)cmdEncoder;
	//
	//    if (nil == _rhiRes->pipelineState)
	//    {
	//        createPipelineHandles(renderPass);
	//    }
	//
	//    MTLRenderPassDescriptor* rpDesc = (__bridge MTLRenderPassDescriptor*)renderPass->handle;
	//    HS_ASSERT(rpDesc != nil, "RenderPass is null");
	//
	//    [encoder pushDebugGroup:@"Forward - Opaque"];
	//
	//    MTLViewport vp = {0.0, 0.0, static_cast<double>(_currentFramebuffer->info.width), static_cast<double>(_currentFramebuffer->info.height), 0.0, 1.0};
	//
	//    [encoder setViewport:vp];
	//
	//    [encoder setRenderPipelineState:_rhiRes->pipelineState];
	//
	//    [encoder setVertexBuffer:_rhiRes->vertexBuffer offset:0 atIndex:0];
	//
	//    [encoder drawPrimitives:MTLPrimitiveTypeTriangle
	//                vertexStart:0
	//                vertexCount:3];
	//
	//    [encoder popDebugGroup];
}

void ForwardOpaquePass::OnAfterRendering()
{

}

void ForwardOpaquePass::createResourceHandles()
{
	RHIContext* rhiContext = _renderer->GetRHIContext();

	struct VSINPUT_BASIC
	{
		Vec4f position; // float4 Pos
		Vec4f color;    // float4 Color
	};

	VSINPUT_BASIC vertices[]{
	{
		{0.5f, -0.5f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f, 1.0f},
	},
	{
		{0.5f, 0.5f, 0.0f, 1.0f},
		{0.0, 1.0f, 0.0f, 1.0f},
	},
	{
		{-0.5f, 0.5f, 0.0f, 1.0f},
		{0.0f, 0.0f, 1.0f, 1.0f},
	},
	};

	const Mesh* fallbackMesh = ObjectManager::GetFallbackMeshCube();
	const auto& pos = fallbackMesh->GetPosition();
	const auto& color = fallbackMesh->GetColor();

	//_vertexBuffer[0] = rhiContext->CreateBuffer(static_cast<const void*>(pos.data()), pos.size() * sizeof(float), EBufferUsage::VERTEX, EBufferMemoryOption::MAPPED);
	//_vertexBuffer[1] = rhiContext->CreateBuffer(static_cast<const void*>(color.data()), color.size() * sizeof(float), EBufferUsage::VERTEX, EBufferMemoryOption::MAPPED);

	//const auto& indices = fallbackMesh.GetIndices();
	//_indexBuffer = rhiContext->CreateBuffer(static_cast<const void*>(indices.data()), indices.size() * sizeof(decltype(indices)), EBufferUsage::INDEX, EBufferMemoryOption::MAPPED);
	//_indexCount = indices.size();
	_vertexBuffer[0] = rhiContext->CreateBuffer("Opaque Test VertexBuffer", vertices, sizeof(vertices), EBufferUsage::VERTEX, EBufferMemoryOption::MAPPED);
#ifdef __APPLE__
	std::string libPath = FileSystem::GetDefaultResourcePath(std::string("Shader/MSL/Basic.metal"));
#elif __WINDOWS__
	std::string libPath = SystemContext::Get()->assetDirectory + std::string("Shaders\\SPV\\Basic.vert.spv");
#endif
	ShaderInfo vsInfo{};
	vsInfo.entryName = "main";
	vsInfo.stage = EShaderStage::VERTEX;
	_vertexShader = rhiContext->CreateShader("Opaque Test Shader", vsInfo, libPath.c_str());
	if (_vertexShader == nullptr)
	{
		HS_LOG(crash, "Shader is nullptr");
	}
	ShaderInfo fsInfo{};
	fsInfo.entryName = "main";
	fsInfo.stage = EShaderStage::FRAGMENT;
#ifdef __WINDOWS__
	libPath = SystemContext::Get()->assetDirectory + std::string("Shaders\\SPV\\Basic.frag.spv");
#endif
	_fragmentShader = rhiContext->CreateShader("Opaque Test Shader", fsInfo, libPath.c_str());
	if (_fragmentShader == nullptr)
	{
		HS_LOG(crash, "Shader is nullptr");
	}
};

void ForwardOpaquePass::createPipelineHandles(RHIRenderPass* renderPass)
{
	RHIContext* rhiContext = _renderer->GetRHIContext();

	DepthStencilStateDescriptor dsDesc{};
	dsDesc.depthTestEnable = false;
	dsDesc.depthWriteEnable = false;

	VertexInputStateDescriptor  viDesc{};
	VertexInputLayoutDescriptor viLayout{};
	viLayout.binding = 0;
	viLayout.stride = sizeof(float) * 4;
	viLayout.stepRate = 1;
	viLayout.useInstancing = false;

	viDesc.layouts.push_back(viLayout);

	VertexInputAttributeDescriptor viAttr{};
	viAttr.location = 0;
	viAttr.binding = 0;
	viAttr.format = EVertexFormat::FLOAT4; // float4 Pos
	viAttr.offset = 0;

	viDesc.attributes.push_back(viAttr);

	viAttr.location = 1;
	viAttr.binding = 0;
	viAttr.format = EVertexFormat::FLOAT4; // float4 Color
	viAttr.offset = 0;

	viDesc.attributes.push_back(viAttr);

	ColorBlendStateDescriptor cbDesc{};
	cbDesc.attachmentCount = renderPass->info.colorAttachmentCount;
	cbDesc.attachments.resize(cbDesc.attachmentCount);
	for (size_t i = 0; i < cbDesc.attachmentCount; i++)
	{
		cbDesc.attachments[i].blendEnable = false;
	}

	RasterizerStateDescriptor rsDesc{};
	rsDesc.cullMode = ECullMode::BACK;
	rsDesc.depthBiasEnable = false;
	rsDesc.frontFace = EFrontFace::CCW;
	rsDesc.polygonMode = EPolygonMode::FILL;
	rsDesc.rasterizerDiscardEnable = false;
	rsDesc.depthClampEnable = false;

	ShaderProgramDescriptor spDesc{};
	spDesc.stages.resize(2);
	spDesc.stages[0] = _vertexShader;
	spDesc.stages[1] = _fragmentShader;

	InputAssemblyStateDescriptor iaDesc{};
	iaDesc.primitiveTopology = EPrimitiveTopology::TRIANGLE_LIST;

	GraphicsPipelineInfo gpInfo{};
	gpInfo.shaderDesc = spDesc;
	gpInfo.inputAssemblyDesc = iaDesc;
	gpInfo.vertexInputDesc = viDesc;
	gpInfo.rasterizerDesc = rsDesc;
	gpInfo.depthStencilDesc = dsDesc;
	gpInfo.colorBlendDesc = cbDesc;

	gpInfo.renderPass = renderPass;

	_gPipeline = rhiContext->CreateGraphicsPipeline("Opaque Test Pipeline", gpInfo);

}

HS_NS_END
