//
//  AtmosphereSkyPass.cpp
//  Engine/Renderer/Atmosphere
//

#include "Engine/Renderer/Atmosphere/AtmosphereSkyPass.h"
#include "Engine/Renderer/Atmosphere/AtmosphereRenderer.h"
#include "Engine/Renderer/RenderPath.h"
#include "Engine/Renderer/RenderTarget.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"

#include "Core/SystemContext.h"

#include "Core/ThirdParty/glm/gtc/matrix_inverse.hpp"
#include "Core/ThirdParty/glm/gtc/matrix_transform.hpp"

HS_NS_BEGIN

AtmosphereSkyPass::AtmosphereSkyPass(const char* name, RenderPath* renderer, AtmosphereRenderer* atmosphereRenderer)
	: RenderPass(name, renderer, ERenderingOrder::SKYBOX)
	, _atmosphereRenderer(atmosphereRenderer)
{
	// Initialize view matrix - camera looking along +Z axis
	_viewMatrix = glm::lookAt(
		_cameraPosition,
		_cameraPosition + glm::vec3(0.0f, 0.0f, 1.0f),  // Looking along +Z
		glm::vec3(0.0f, 1.0f, 0.0f)                      // Up is +Y
	);

	// Initialize projection matrix with Vulkan Y-flip
	_projectionMatrix = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 100000.0f);
	_projectionMatrix[1][1] *= -1.0f;  // Vulkan Y-flip

	// Initialize uniforms
	_uniforms = {};
	_uniforms.exposure = 10.0f;
	_uniforms.whitePoint = glm::vec3(1.08241f, 0.96756f, 0.95003f);
	_uniforms.sunAngularRadius = 0.00935f;
	_uniforms.earthCenter = glm::vec3(0.0f, -6360000.0f, 0.0f); // Earth center below camera
}

AtmosphereSkyPass::~AtmosphereSkyPass()
{
	destroyResources();
}

void AtmosphereSkyPass::OnBeforeRendering(uint32_t submitIndex)
{
	this->frameIndex = submitIndex;

	// Create resources on first frame
	if (!_resourcesCreated)
	{
		createResources();
		_resourcesCreated = true;
	}

	// Update uniforms if dirty
	if (_uniformsDirty)
	{
		updateUniforms();
		_uniformsDirty = false;
	}
}

void AtmosphereSkyPass::Configure(RenderTarget* renderTarget)
{
	_currentRenderTarget = renderTarget;

	const RenderTargetInfo& rtInfo = renderTarget->GetInfo();

	_renderPassInfo.colorAttachmentCount = 1;
	Attachment ca{};
	ca.format = rtInfo.colorTextureInfos[0].format;
	ca.clearValue = ClearValue(0.0f, 0.0f, 0.0f, 1.0f);
	ca.isDepthStencil = false;
	ca.loadAction = ELoadAction::CLEAR;
	ca.storeAction = EStoreAction::STORE;
	_renderPassInfo.colorAttachments.clear();
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

void AtmosphereSkyPass::Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass)
{
	if (!_atmosphereRenderer->IsPrecomputed())
	{
		HS_LOG(warning, "Atmosphere not precomputed, skipping sky rendering");
		return;
	}

	// Check if resources were created successfully
	if (_uniformBuffer == nullptr || _resourceSet == nullptr)
	{
		HS_LOG(warning, "Sky pass resources not available, skipping rendering");
		return;
	}

	// Create pipeline if not yet created
	if (_pipeline == nullptr)
	{
		createPipeline(renderPass);
	}

	if (_pipeline == nullptr)
	{
		HS_LOG(error, "Failed to create sky pipeline");
		return;
	}

	// Update uniform buffer
	commandBuffer->UpdateBuffer(_uniformBuffer, 0, &_uniforms, sizeof(_uniforms));

	// Get framebuffer
	RHIFramebuffer* framebuffer = _renderer->GetHandleCache()->GetFramebuffer(renderPass, _currentRenderTarget);

	float debugColor[4]{ 0.3f, 0.5f, 0.9f, 1.0f };
	commandBuffer->PushDebugMark("Atmosphere Sky Pass", debugColor);

	Area area = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
	commandBuffer->BeginRenderPass(renderPass, framebuffer, area);

	commandBuffer->BindPipeline(_pipeline);

	// Bind resource set
	commandBuffer->BindResourceSet(_resourceSet);

	commandBuffer->SetViewport(Viewport{
		0.0f, 0.0f,
		static_cast<float>(framebuffer->info.width),
		static_cast<float>(framebuffer->info.height),
		0.0f, 1.0f
	});

	commandBuffer->SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);

	// Bind fullscreen quad vertex buffer
	uint32 offset = 0;
	commandBuffer->BindVertexBuffers(&_fullscreenQuadVB, &offset, 1);

	// Draw fullscreen triangle (3 vertices)
	commandBuffer->DrawArrays(0, 3, 1);

	commandBuffer->EndRenderPass();
	commandBuffer->PopDebugMark();
}

void AtmosphereSkyPass::OnAfterRendering()
{
}

void AtmosphereSkyPass::Clear()
{
	destroyResources();
}

void AtmosphereSkyPass::SetViewProjectionMatrix(const glm::mat4& view, const glm::mat4& projection)
{
	_viewMatrix = view;
	_projectionMatrix = projection;
	_uniformsDirty = true;
}

void AtmosphereSkyPass::SetCameraPosition(const glm::vec3& position)
{
	_cameraPosition = position;
	_uniformsDirty = true;
}

void AtmosphereSkyPass::SetSunDirection(const glm::vec3& direction)
{
	_sunDirection = glm::normalize(direction);
	_uniformsDirty = true;
}

void AtmosphereSkyPass::createResources()
{
	RHIContext* context = _renderer->GetRHIContext();

	// Check if atmosphere textures are available
	if (_atmosphereRenderer->GetTransmittanceTexture() == nullptr ||
		_atmosphereRenderer->GetScatteringTexture() == nullptr ||
		_atmosphereRenderer->GetIrradianceTexture() == nullptr)
	{
		HS_LOG(warning, "Atmosphere textures not available, skipping resource creation");
		return;
	}

	// Create fullscreen triangle (uses NDC, no vertex transformation needed)
	// Triangle covers [-1,-1] to [3,3] in clip space
	float vertices[] = {
		-1.0f, -1.0f, 0.0f,  // Bottom-left
		 3.0f, -1.0f, 0.0f,  // Bottom-right (extends past viewport)
		-1.0f,  3.0f, 0.0f   // Top-left (extends past viewport)
	};
	_fullscreenQuadVB = context->CreateBuffer("Sky Fullscreen Triangle VB",
		vertices, sizeof(vertices),
		EBufferUsage::VERTEX, EBufferMemoryOption::STATIC);

	// Create uniform buffer
	_uniformBuffer = context->CreateBuffer("Sky Uniforms",
		nullptr, sizeof(AtmosphereRenderer::SkyUniforms),
		EBufferUsage::UNIFORM, EBufferMemoryOption::MAPPED);

	// Load shaders
#ifdef __APPLE__
	std::string vsPath = SystemContext::Get()->assetDirectory + "Shaders/Sky.vert.metal";
	std::string fsPath = SystemContext::Get()->assetDirectory + "Shaders/Sky.frag.metal";
	const char* entryVS = "VertexMain";
	const char* entryFS = "FragmentMain";
#elif __WINDOWS__
	std::string vsPath = SystemContext::Get()->assetDirectory + "Shaders\\Sky.vert.spv";
	std::string fsPath = SystemContext::Get()->assetDirectory + "Shaders\\Sky.frag.spv";
	const char* entryVS = "main";
	const char* entryFS = "main";
#endif

	ShaderInfo vsInfo{};
	vsInfo.stage = EShaderStage::VERTEX;
	vsInfo.entryName = entryVS;
	_vertexShader = context->CreateShader("Sky Vertex Shader", vsInfo, vsPath.c_str());

	ShaderInfo fsInfo{};
	fsInfo.stage = EShaderStage::FRAGMENT;
	fsInfo.entryName = entryFS;
	_fragmentShader = context->CreateShader("Sky Fragment Shader", fsInfo, fsPath.c_str());

	if (_vertexShader == nullptr || _fragmentShader == nullptr)
	{
		HS_LOG(error, "Failed to load sky shaders");
		return;
	}

	// Create resource layout
	// Binding 0: Uniform buffer (SkyUniforms)
	// Binding 1: Transmittance texture
	// Binding 2: Scattering texture (3D)
	// Binding 3: Irradiance texture
	// Binding 4: Single Mie scattering texture (3D)
	// Binding 5: Sampler

	std::vector<ResourceBinding> bindings(6);

	// Uniform buffer
	bindings[0].type = EResourceType::UNIFORM_BUFFER;
	bindings[0].stage = EShaderStage::VERTEX | EShaderStage::FRAGMENT;
	bindings[0].binding = 0;
	bindings[0].arrayCount = 1;
	bindings[0].name = "SkyUniforms";
	bindings[0].resource.buffers.push_back(_uniformBuffer);

	// Transmittance texture
	bindings[1].type = EResourceType::SAMPLED_IMAGE;
	bindings[1].stage = EShaderStage::FRAGMENT;
	bindings[1].binding = 1;
	bindings[1].arrayCount = 1;
	bindings[1].name = "TransmittanceTexture";
	bindings[1].resource.textures.push_back(_atmosphereRenderer->GetTransmittanceTexture());

	// Scattering texture (3D)
	bindings[2].type = EResourceType::SAMPLED_IMAGE;
	bindings[2].stage = EShaderStage::FRAGMENT;
	bindings[2].binding = 2;
	bindings[2].arrayCount = 1;
	bindings[2].name = "ScatteringTexture";
	bindings[2].resource.textures.push_back(_atmosphereRenderer->GetScatteringTexture());

	// Irradiance texture
	bindings[3].type = EResourceType::SAMPLED_IMAGE;
	bindings[3].stage = EShaderStage::FRAGMENT;
	bindings[3].binding = 3;
	bindings[3].arrayCount = 1;
	bindings[3].name = "IrradianceTexture";
	bindings[3].resource.textures.push_back(_atmosphereRenderer->GetIrradianceTexture());

	// Single Mie scattering texture (3D)
	bindings[4].type = EResourceType::SAMPLED_IMAGE;
	bindings[4].stage = EShaderStage::FRAGMENT;
	bindings[4].binding = 4;
	bindings[4].arrayCount = 1;
	bindings[4].name = "SingleMieScatteringTexture";
	RHITexture* singleMie = _atmosphereRenderer->GetSingleMieScatteringTexture();
	bindings[4].resource.textures.push_back(singleMie ? singleMie : _atmosphereRenderer->GetScatteringTexture());

	// Sampler
	bindings[5].type = EResourceType::SAMPLER;
	bindings[5].stage = EShaderStage::FRAGMENT;
	bindings[5].binding = 5;
	bindings[5].arrayCount = 1;
	bindings[5].name = "LUTSampler";
	bindings[5].resource.samplers.push_back(_atmosphereRenderer->GetLUTSampler());

	_resourceLayout = context->CreateResourceLayout("Sky Resource Layout", bindings.data(), static_cast<uint32>(bindings.size()));

	// Create resource set from layout
	_resourceSet = context->CreateResourceSet("Sky Resource Set", _resourceLayout);
}

void AtmosphereSkyPass::destroyResources()
{
	RHIContext* context = _renderer->GetRHIContext();

	if (_resourceSet != nullptr)
	{
		context->DestroyResourceSet(_resourceSet);
		_resourceSet = nullptr;
	}

	if (_resourceLayout != nullptr)
	{
		context->DestroyResourceLayout(_resourceLayout);
		_resourceLayout = nullptr;
	}

	if (_pipeline != nullptr)
	{
		context->DestroyGraphicsPipeline(_pipeline);
		_pipeline = nullptr;
	}

	if (_uniformBuffer != nullptr)
	{
		context->DestroyBuffer(_uniformBuffer);
		_uniformBuffer = nullptr;
	}

	if (_fullscreenQuadVB != nullptr)
	{
		context->DestroyBuffer(_fullscreenQuadVB);
		_fullscreenQuadVB = nullptr;
	}

	if (_vertexShader != nullptr)
	{
		context->DestroyShader(_vertexShader);
		_vertexShader = nullptr;
	}

	if (_fragmentShader != nullptr)
	{
		context->DestroyShader(_fragmentShader);
		_fragmentShader = nullptr;
	}

	_resourcesCreated = false;
}

void AtmosphereSkyPass::createPipeline(RHIRenderPass* renderPass)
{
	if (_vertexShader == nullptr || _fragmentShader == nullptr)
	{
		HS_LOG(error, "Shaders not loaded, cannot create pipeline");
		return;
	}

	RHIContext* context = _renderer->GetRHIContext();

	// Shader program
	ShaderProgramDescriptor spDesc{};
	spDesc.stages.resize(2);
	spDesc.stages[0] = _vertexShader;
	spDesc.stages[1] = _fragmentShader;

	// Vertex input - simple position only
	VertexInputStateDescriptor viDesc{};
	VertexInputLayoutDescriptor viLayout{};
	viLayout.binding = 0;
	viLayout.stride = sizeof(float) * 3;
	viLayout.stepRate = 1;
	viLayout.useInstancing = false;
	viDesc.layouts.push_back(viLayout);

	VertexInputAttributeDescriptor viAttr{};
	viAttr.location = 0;
	viAttr.binding = 0;
	viAttr.format = EVertexFormat::FLOAT3;
	viAttr.offset = 0;
	viDesc.attributes.push_back(viAttr);

	// Input assembly
	InputAssemblyStateDescriptor iaDesc{};
	iaDesc.primitiveTopology = EPrimitiveTopology::TRIANGLE_LIST;
	iaDesc.isRestartEnable = false;

	// Rasterizer - no culling for fullscreen triangle
	RasterizerStateDescriptor rsDesc{};
	rsDesc.cullMode = ECullMode::NONE;
	rsDesc.frontFace = EFrontFace::CCW;
	rsDesc.polygonMode = EPolygonMode::FILL;
	rsDesc.depthClampEnable = false;
	rsDesc.rasterizerDiscardEnable = false;
	rsDesc.depthBiasEnable = false;

	// Depth stencil - disabled (no depth buffer in render target)
	DepthStencilStateDescriptor dsDesc{};
	dsDesc.depthTestEnable = false;
	dsDesc.depthWriteEnable = false;
	dsDesc.depthCompareOp = ECompareOp::ALWAYS;
	dsDesc.stencilTestEnable = false;

	// Color blend - no blending
	ColorBlendStateDescriptor cbDesc{};
	cbDesc.attachmentCount = renderPass->info.colorAttachmentCount;
	cbDesc.attachments.resize(cbDesc.attachmentCount);
	for (size_t i = 0; i < cbDesc.attachmentCount; i++)
	{
		cbDesc.attachments[i].blendEnable = false;
		cbDesc.attachments[i].writeMask = 0xF;
	}

	// Create pipeline
	GraphicsPipelineInfo gpInfo{};
	gpInfo.shaderDesc = spDesc;
	gpInfo.inputAssemblyDesc = iaDesc;
	gpInfo.vertexInputDesc = viDesc;
	gpInfo.rasterizerDesc = rsDesc;
	gpInfo.depthStencilDesc = dsDesc;
	gpInfo.colorBlendDesc = cbDesc;
	gpInfo.resourceLayout = _resourceLayout;
	gpInfo.renderPass = renderPass;

	_pipeline = context->CreateGraphicsPipeline("Atmosphere Sky Pipeline", gpInfo);
}

void AtmosphereSkyPass::updateUniforms()
{
	// Compute inverse view-projection matrix
	glm::mat4 viewProj = _projectionMatrix * _viewMatrix;
	_uniforms.inverseViewProjection = glm::inverse(viewProj);

	// Camera position in world coordinates, earth center below
	// camera (planet-centered) = cameraPosition - earthCenter
	const AtmosphereParameters& params = _atmosphereRenderer->GetParameters();
	_uniforms.earthCenter = glm::vec3(0.0f, -params.bottomRadius, 0.0f);
	_uniforms.cameraPosition = _cameraPosition;

	// Sun direction and parameters
	_uniforms.sunDirection = _sunDirection;
	_uniforms.sunAngularRadius = params.sunAngularRadius;

	// Tone mapping
	_uniforms.exposure = _atmosphereRenderer->GetExposure();
	_uniforms.whitePoint = _atmosphereRenderer->GetWhitePoint();
}

HS_NS_END
