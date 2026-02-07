#include "Engine/Renderer/RenderPass/ForwardOpaquePass.h"

#include "Core/HAL/FileSystem.h"
#include "Core/Log.h"
#include "Core/SystemContext.h"

#include "Renderer/RenderPath.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"
#include "RHI/CommandHandle.h"
#include "RHI/RHIContext.h"

#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Model.h"
#include "Resource/ResourceDefinition.h"

#include "Engine/Camera.h"

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

	_renderPassInfo = {};

	_renderPassInfo.colorAttachmentCount = 1;
	Attachment ca{};
	ca.format = rtInfo.colorTextureInfos[0].format;
	ca.clearValue = ClearValue(0.2f, 0.5f, 0.5f, 1.0f);
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
	// No-op: use the 3-arg version
}

void ForwardOpaquePass::Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass, const RenderParameter& param)
{
	if (param.cameras.empty() || param.models.empty())
	{
		return;
	}

	if (nullptr == _gPipeline)
	{
		createPipelineHandles(renderPass);
	}

	// Update camera and fill PerView UBO
	Camera* camera = param.cameras[0];
	camera->Update();

	PerView perViewData{};
	perViewData.viewMatrix                  = camera->GetViewMatrix();
	perViewData.projectionMatrix            = camera->GetProjectionMatrix();
	perViewData.viewProjectionMatrix        = camera->GetViewProjectionMatrix();
	perViewData.inverseViewMatrix           = camera->GetInverseViewMatrix();
	perViewData.inverseProjectionMatrix     = camera->GetInverseProjectionMatrix();
	perViewData.inverseViewProjectionMatrix = camera->GetInverseViewProjectionMatrix();
	perViewData.cameraPosition              = glm::vec4(camera->GetPosition(), 1.0f);

	commandBuffer->UpdateBuffer(_perViewBuffer, 0, &perViewData, sizeof(PerView));

	// Begin render pass
	RHIFramebuffer* framebuffer = _renderer->GetHandleCache()->GetFramebuffer(renderPass, _currentRenderTarget);

	float debugColor[4]{ 0.2f, 0.5f, 0.8f, 1.0f };
	commandBuffer->PushDebugMark("Opaque Pass", debugColor);

	Area area = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
	commandBuffer->BeginRenderPass(renderPass, framebuffer, area);
	commandBuffer->BindPipeline(_gPipeline);
	commandBuffer->SetViewport(Viewport{ 0.0f, 0.0f, static_cast<float>(framebuffer->info.width), static_cast<float>(framebuffer->info.height), 0.0f, 1.0f });
	commandBuffer->SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);

	RHIContext* rhiContext = _renderer->GetRHIContext();

	for (auto* model : param.models)
	{
		Mesh* mesh = model->GetMesh();
		if (!mesh) continue;

		const auto& positions = mesh->GetPosition();
		const auto& normals   = mesh->GetNormal();
		const auto& indices   = mesh->GetIndices();

		if (positions.empty() || indices.empty()) continue;

		// Create interleaved VB (lazy, cached per mesh)
		if (_meshVertexBuffers.find(mesh) == _meshVertexBuffers.end())
		{
			uint32 vertexCount = mesh->GetVertexCount();
			bool hasNormals = mesh->HasNormals();

			// Interleave: [pos.x, pos.y, pos.z, n.x, n.y, n.z] per vertex
			std::vector<float> interleavedData(vertexCount * 6);
			for (uint32 v = 0; v < vertexCount; v++)
			{
				interleavedData[v * 6 + 0] = positions[v * 3 + 0];
				interleavedData[v * 6 + 1] = positions[v * 3 + 1];
				interleavedData[v * 6 + 2] = positions[v * 3 + 2];
				interleavedData[v * 6 + 3] = hasNormals ? normals[v * 3 + 0] : 0.0f;
				interleavedData[v * 6 + 4] = hasNormals ? normals[v * 3 + 1] : 1.0f;
				interleavedData[v * 6 + 5] = hasNormals ? normals[v * 3 + 2] : 0.0f;
			}

			_meshVertexBuffers[mesh] = rhiContext->CreateBuffer(
				"Mesh VB", interleavedData.data(),
				interleavedData.size() * sizeof(float),
				EBufferUsage::VERTEX, EBufferMemoryOption::MAPPED);

			_meshIndexBuffers[mesh] = rhiContext->CreateBuffer(
				"Mesh IB", indices.data(),
				indices.size() * sizeof(uint32),
				EBufferUsage::INDEX, EBufferMemoryOption::MAPPED);

			_meshIndexCounts[mesh] = static_cast<uint32>(indices.size());
		}

		// Fill PerDraw UBO
		PerDraw perDrawData{};
		perDrawData.modelMatrix        = model->GetWorldMatrix();
		perDrawData.inverseModelMatrix = model->GetInverseWorldMatrix();
		commandBuffer->UpdateBuffer(_perDrawBuffer, 0, &perDrawData, sizeof(PerDraw));

		// Bind vertex buffer
		uint32 vbOffset = 0;
		const RHIBuffer* vb = _meshVertexBuffers[mesh];
		commandBuffer->BindVertexBuffers(&vb, &vbOffset, 1);

		// Bind resource set (uniforms)
		commandBuffer->BindResourceSet(_resourceSet);

		// Bind index buffer and draw
		commandBuffer->BindIndexBuffer(_meshIndexBuffers[mesh]);
		commandBuffer->DrawIndexed(0, _meshIndexCounts[mesh], 1, 0);
	}

	commandBuffer->EndRenderPass();
	commandBuffer->PopDebugMark();
}

void ForwardOpaquePass::OnAfterRendering()
{
}

void ForwardOpaquePass::createResourceHandles()
{
	RHIContext* rhiContext = _renderer->GetRHIContext();

	// Load Blinn-Phong shaders
#ifdef __APPLE__
	std::string libPath = SystemContext::Get()->assetDirectory + "Shaders/BlinnPhong.vert.metal";
	const char* entryName = "VertexMain";
#elif __WINDOWS__
	SystemContext* systemContext = SystemContext::Get();
	std::string libPath = systemContext->assetDirectory + "Shaders\\BlinnPhong.vert.spv";
	const char* entryName = "main";
#endif

	ShaderInfo vsInfo{};
	vsInfo.entryName = entryName;
	vsInfo.stage = EShaderStage::VERTEX;
	_vertexShader = rhiContext->CreateShader("BlinnPhong Vertex Shader", vsInfo, libPath.c_str());
	if (_vertexShader == nullptr)
	{
		HS_LOG(crash, "BlinnPhong vertex shader is nullptr");
	}

#ifdef __APPLE__
	libPath = SystemContext::Get()->assetDirectory + std::string("Shaders/BlinnPhong.frag.metal");
	entryName = "FragmentMain";
#elif __WINDOWS__
	libPath = SystemContext::Get()->assetDirectory + std::string("Shaders\\BlinnPhong.frag.spv");
#endif

	ShaderInfo fsInfo{};
	fsInfo.entryName = entryName;
	fsInfo.stage = EShaderStage::FRAGMENT;
	_fragmentShader = rhiContext->CreateShader("BlinnPhong Fragment Shader", fsInfo, libPath.c_str());
	if (_fragmentShader == nullptr)
	{
		HS_LOG(crash, "BlinnPhong fragment shader is nullptr");
	}

	// Create uniform buffers
	PerView perViewZero{};
	_perViewBuffer = rhiContext->CreateBuffer("PerView UBO", &perViewZero, sizeof(PerView),
		EBufferUsage::UNIFORM, EBufferMemoryOption::DYNAMIC);

	PerDraw perDrawZero{};
	_perDrawBuffer = rhiContext->CreateBuffer("PerDraw UBO", &perDrawZero, sizeof(PerDraw),
		EBufferUsage::UNIFORM, EBufferMemoryOption::DYNAMIC);

	// Create resource layout: perView@1 VERTEX, perDraw@2 VERTEX, perView@1 FRAGMENT
	ResourceBinding bindings[3]{};

	// binding 0: perView for vertex stage at buffer index 1
	bindings[0].type       = EResourceType::UNIFORM_BUFFER;
	bindings[0].stage      = EShaderStage::VERTEX;
	bindings[0].binding    = 1;
	bindings[0].arrayCount = 1;
	bindings[0].resource.buffers.push_back(_perViewBuffer);
	bindings[0].resource.offsets.push_back(0);

	// binding 1: perDraw for vertex stage at buffer index 2
	bindings[1].type       = EResourceType::UNIFORM_BUFFER;
	bindings[1].stage      = EShaderStage::VERTEX;
	bindings[1].binding    = 2;
	bindings[1].arrayCount = 1;
	bindings[1].resource.buffers.push_back(_perDrawBuffer);
	bindings[1].resource.offsets.push_back(0);

	// binding 2: perView for fragment stage at buffer index 1
	bindings[2].type       = EResourceType::UNIFORM_BUFFER;
	bindings[2].stage      = EShaderStage::FRAGMENT;
	bindings[2].binding    = 1;
	bindings[2].arrayCount = 1;
	bindings[2].resource.buffers.push_back(_perViewBuffer);
	bindings[2].resource.offsets.push_back(0);

	_resourceLayout = rhiContext->CreateResourceLayout("BlinnPhong Layout", bindings, 3);

	_resourceSet = rhiContext->CreateResourceSet("BlinnPhong ResourceSet", _resourceLayout);
	_resourceSet->layouts.push_back(_resourceLayout);
}

void ForwardOpaquePass::createPipelineHandles(RHIRenderPass* renderPass)
{
	RHIContext* rhiContext = _renderer->GetRHIContext();

	DepthStencilStateDescriptor dsDesc{};
	dsDesc.depthTestEnable  = true;
	dsDesc.depthWriteEnable = true;
	dsDesc.depthCompareOp   = ECompareOp::LESS;

	VertexInputStateDescriptor viDesc{};
	VertexInputLayoutDescriptor viLayout{};
	viLayout.binding       = 0;
	viLayout.stride        = sizeof(float) * 6; // float3 pos + float3 normal
	viLayout.stepRate      = 1;
	viLayout.useInstancing = false;
	viDesc.layouts.push_back(viLayout);

	VertexInputAttributeDescriptor viAttr{};
	// location 0: position (float3) at offset 0
	viAttr.location = 0;
	viAttr.binding  = 0;
	viAttr.format   = EVertexFormat::FLOAT3;
	viAttr.offset   = 0;
	viDesc.attributes.push_back(viAttr);

	// location 1: normal (float3) at offset 12
	viAttr.location = 1;
	viAttr.binding  = 0;
	viAttr.format   = EVertexFormat::FLOAT3;
	viAttr.offset   = sizeof(float) * 3;
	viDesc.attributes.push_back(viAttr);

	ColorBlendStateDescriptor cbDesc{};
	cbDesc.attachmentCount = renderPass->info.colorAttachmentCount;
	cbDesc.attachments.resize(cbDesc.attachmentCount);
	for (size_t i = 0; i < cbDesc.attachmentCount; i++)
	{
		cbDesc.attachments[i].blendEnable = false;
	}

	RasterizerStateDescriptor rsDesc{};
	rsDesc.cullMode                = ECullMode::BACK;
	rsDesc.depthBiasEnable         = false;
	rsDesc.frontFace               = EFrontFace::CCW;
	rsDesc.polygonMode             = EPolygonMode::FILL;
	rsDesc.rasterizerDiscardEnable = false;
	rsDesc.depthClampEnable        = false;

	ShaderProgramDescriptor spDesc{};
	spDesc.stages.resize(2);
	spDesc.stages[0] = _vertexShader;
	spDesc.stages[1] = _fragmentShader;

	InputAssemblyStateDescriptor iaDesc{};
	iaDesc.primitiveTopology = EPrimitiveTopology::TRIANGLE_LIST;

	GraphicsPipelineInfo gpInfo{};
	gpInfo.shaderDesc       = spDesc;
	gpInfo.inputAssemblyDesc = iaDesc;
	gpInfo.vertexInputDesc  = viDesc;
	gpInfo.rasterizerDesc   = rsDesc;
	gpInfo.depthStencilDesc = dsDesc;
	gpInfo.colorBlendDesc   = cbDesc;
	gpInfo.renderPass       = renderPass;

	_gPipeline = rhiContext->CreateGraphicsPipeline("BlinnPhong Pipeline", gpInfo);
}

HS_NS_END
