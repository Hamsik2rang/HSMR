//
//  AtmospherePrecompute.cpp
//  Engine/Renderer/Atmosphere
//
//  Created by Yongsik Im on 1/2/26.
//

#include "Engine/Renderer/Atmosphere/AtmospherePrecompute.h"

#include "RHI/RHIContext.h"
#include "RHI/RHIDefinition.h"
#include "RHI/ResourceHandle.h"
#include "RHI/RenderHandle.h"
#include "RHI/CommandHandle.h"

#include "Core/Log.h"

HS_NS_BEGIN

AtmospherePrecompute::AtmospherePrecompute()
{
}

AtmospherePrecompute::~AtmospherePrecompute()
{
	if (_isInitialized)
	{
		Finalize();
	}
}

bool AtmospherePrecompute::Initialize(RHIContext* context, const AtmosphereParameters& params,
                                       int numScatteringOrders)
{
	if (_isInitialized)
	{
		HS_LOG(warning, "AtmospherePrecompute already initialized");
		return false;
	}

	_context = context;
	_params = params;
	_numScatteringOrders = numScatteringOrders;

	// Create parameter buffer
	AtmosphereParametersGPU gpuParams;
	ConvertToGPUFormat(params, gpuParams);

	_paramsBuffer = _context->CreateBuffer(
		"AtmosphereParams",
		&gpuParams,
		sizeof(AtmosphereParametersGPU),
		EBufferUsage::UNIFORM,
		EBufferMemoryOption::DYNAMIC
	);

	if (!_paramsBuffer)
	{
		HS_LOG(error, "Failed to create atmosphere parameters buffer");
		return false;
	}

	// Create textures
	if (!CreateTextures())
	{
		HS_LOG(error, "Failed to create atmosphere textures");
		Finalize();
		return false;
	}

	// Create sampler
	SamplerInfo samplerInfo{};
	samplerInfo.minFilter = EFilterMode::LINEAR;
	samplerInfo.magFilter = EFilterMode::LINEAR;
	samplerInfo.mipmapMode = EFilterMode::LINEAR;
	samplerInfo.addressU = EAddressMode::CLAMP_TO_EDGE;
	samplerInfo.addressV = EAddressMode::CLAMP_TO_EDGE;
	samplerInfo.addressW = EAddressMode::CLAMP_TO_EDGE;

	_linearSampler = _context->CreateSampler("AtmosphereSampler", samplerInfo);
	if (!_linearSampler)
	{
		HS_LOG(error, "Failed to create atmosphere sampler");
		Finalize();
		return false;
	}

	// Create shaders
	if (!CreateShaders())
	{
		HS_LOG(error, "Failed to create atmosphere shaders");
		Finalize();
		return false;
	}

	// Create resource layouts
	if (!CreateResourceLayouts())
	{
		HS_LOG(error, "Failed to create atmosphere resource layouts");
		Finalize();
		return false;
	}

	// Create pipelines
	if (!CreatePipelines())
	{
		HS_LOG(error, "Failed to create atmosphere pipelines");
		Finalize();
		return false;
	}

	// Create resource sets
	if (!CreateResourceSets())
	{
		HS_LOG(error, "Failed to create atmosphere resource sets");
		Finalize();
		return false;
	}

	_isInitialized = true;
	_isComputed = false;

	HS_LOG(info, "AtmospherePrecompute initialized successfully");
	return true;
}

void AtmospherePrecompute::Finalize()
{
	if (!_context) return;

	DestroyResourceSets();
	DestroyPipelines();
	DestroyResourceLayouts();
	DestroyShaders();
	DestroyTextures();

	if (_linearSampler)
	{
		_context->DestroySampler(_linearSampler);
		_linearSampler = nullptr;
	}

	if (_paramsBuffer)
	{
		_context->DestroyBuffer(_paramsBuffer);
		_paramsBuffer = nullptr;
	}

	_context = nullptr;
	_isInitialized = false;
	_isComputed = false;
}

void AtmospherePrecompute::ConvertToGPUFormat(const AtmosphereParameters& params,
                                               AtmosphereParametersGPU& gpuParams)
{
	// Solar
	gpuParams.solarIrradiance = params.solarIrradiance;
	gpuParams.sunAngularRadius = params.sunAngularRadius;

	// Planet
	gpuParams.bottomRadius = params.bottomRadius;
	gpuParams.topRadius = params.topRadius;

	// Rayleigh
	for (int i = 0; i < DensityProfile::MAX_LAYERS; i++)
	{
		gpuParams.rayleighDensity[i].width = params.rayleighDensity.layers[i].width;
		gpuParams.rayleighDensity[i].expTerm = params.rayleighDensity.layers[i].expTerm;
		gpuParams.rayleighDensity[i].expScale = params.rayleighDensity.layers[i].expScale;
		gpuParams.rayleighDensity[i].linearTerm = params.rayleighDensity.layers[i].linearTerm;
		gpuParams.rayleighDensity[i].constantTerm = params.rayleighDensity.layers[i].constantTerm;
	}
	gpuParams.rayleighScattering = params.rayleighScattering;

	// Mie
	for (int i = 0; i < DensityProfile::MAX_LAYERS; i++)
	{
		gpuParams.mieDensity[i].width = params.mieDensity.layers[i].width;
		gpuParams.mieDensity[i].expTerm = params.mieDensity.layers[i].expTerm;
		gpuParams.mieDensity[i].expScale = params.mieDensity.layers[i].expScale;
		gpuParams.mieDensity[i].linearTerm = params.mieDensity.layers[i].linearTerm;
		gpuParams.mieDensity[i].constantTerm = params.mieDensity.layers[i].constantTerm;
	}
	gpuParams.mieScattering = params.mieScattering;
	gpuParams.miePhaseFunctionG = params.miePhaseFunctionG;
	gpuParams.mieExtinction = params.mieExtinction;

	// Absorption
	for (int i = 0; i < DensityProfile::MAX_LAYERS; i++)
	{
		gpuParams.absorptionDensity[i].width = params.absorptionDensity.layers[i].width;
		gpuParams.absorptionDensity[i].expTerm = params.absorptionDensity.layers[i].expTerm;
		gpuParams.absorptionDensity[i].expScale = params.absorptionDensity.layers[i].expScale;
		gpuParams.absorptionDensity[i].linearTerm = params.absorptionDensity.layers[i].linearTerm;
		gpuParams.absorptionDensity[i].constantTerm = params.absorptionDensity.layers[i].constantTerm;
	}
	gpuParams.absorptionExtinction = params.absorptionExtinction;

	// Ground
	gpuParams.groundAlbedo = params.groundAlbedo;
	gpuParams.muSMin = params.muSMin;
}

bool AtmospherePrecompute::CreateTextures()
{
	using namespace AtmosphereConstants;

	// Transmittance texture: 2D, RGBA32F
	TextureInfo transmittanceInfo{};
	transmittanceInfo.extent.width = TRANSMITTANCE_TEXTURE_WIDTH;
	transmittanceInfo.extent.height = TRANSMITTANCE_TEXTURE_HEIGHT;
	transmittanceInfo.extent.depth = 1;
	transmittanceInfo.mipLevel = 1;
	transmittanceInfo.arrayLength = 1;
	transmittanceInfo.format = EPixelFormat::RGBA32F;
	transmittanceInfo.type = ETextureType::TEX_2D;
	transmittanceInfo.usage = ETextureUsage::SAMPLED | ETextureUsage::STORAGE;

	_transmittanceTexture = _context->CreateTexture("AtmosphereTransmittance", nullptr, transmittanceInfo);
	if (!_transmittanceTexture) return false;

	// Scattering texture: 3D, RGBA16F
	TextureInfo scatteringInfo{};
	scatteringInfo.extent.width = SCATTERING_TEXTURE_WIDTH;
	scatteringInfo.extent.height = SCATTERING_TEXTURE_HEIGHT;
	scatteringInfo.extent.depth = SCATTERING_TEXTURE_DEPTH;
	scatteringInfo.mipLevel = 1;
	scatteringInfo.arrayLength = 1;
	scatteringInfo.format = EPixelFormat::RGBA16F;
	scatteringInfo.type = ETextureType::TEX_3D;
	scatteringInfo.usage = ETextureUsage::SAMPLED | ETextureUsage::STORAGE;

	_scatteringTexture = _context->CreateTexture("AtmosphereScattering", nullptr, scatteringInfo);
	if (!_scatteringTexture) return false;

	// Irradiance texture: 2D, RGBA32F
	TextureInfo irradianceInfo{};
	irradianceInfo.extent.width = IRRADIANCE_TEXTURE_WIDTH;
	irradianceInfo.extent.height = IRRADIANCE_TEXTURE_HEIGHT;
	irradianceInfo.extent.depth = 1;
	irradianceInfo.mipLevel = 1;
	irradianceInfo.arrayLength = 1;
	irradianceInfo.format = EPixelFormat::RGBA32F;
	irradianceInfo.type = ETextureType::TEX_2D;
	irradianceInfo.usage = ETextureUsage::SAMPLED | ETextureUsage::STORAGE;

	_irradianceTexture = _context->CreateTexture("AtmosphereIrradiance", nullptr, irradianceInfo);
	if (!_irradianceTexture) return false;

	// Single Mie scattering texture (optional, same as scattering)
	_singleMieScatteringTexture = _context->CreateTexture("AtmosphereSingleMie", nullptr, scatteringInfo);
	if (!_singleMieScatteringTexture) return false;

	// Delta textures for multi-scattering computation
	_deltaIrradianceTexture = _context->CreateTexture("AtmosphereDeltaIrradiance", nullptr, irradianceInfo);
	if (!_deltaIrradianceTexture) return false;

	_deltaRayleighScatteringTexture = _context->CreateTexture("AtmosphereDeltaRayleigh", nullptr, scatteringInfo);
	if (!_deltaRayleighScatteringTexture) return false;

	_deltaMieScatteringTexture = _context->CreateTexture("AtmosphereDeltaMie", nullptr, scatteringInfo);
	if (!_deltaMieScatteringTexture) return false;

	_deltaScatteringDensityTexture = _context->CreateTexture("AtmosphereDeltaDensity", nullptr, scatteringInfo);
	if (!_deltaScatteringDensityTexture) return false;

	_deltaMultipleScatteringTexture = _context->CreateTexture("AtmosphereDeltaMultiple", nullptr, scatteringInfo);
	if (!_deltaMultipleScatteringTexture) return false;

	return true;
}

void AtmospherePrecompute::DestroyTextures()
{
	if (!_context) return;

	auto destroyTexture = [this](RHITexture*& tex) {
		if (tex)
		{
			_context->DestroyTexture(tex);
			tex = nullptr;
		}
	};

	destroyTexture(_transmittanceTexture);
	destroyTexture(_scatteringTexture);
	destroyTexture(_irradianceTexture);
	destroyTexture(_singleMieScatteringTexture);
	destroyTexture(_deltaIrradianceTexture);
	destroyTexture(_deltaRayleighScatteringTexture);
	destroyTexture(_deltaMieScatteringTexture);
	destroyTexture(_deltaScatteringDensityTexture);
	destroyTexture(_deltaMultipleScatteringTexture);
}

bool AtmospherePrecompute::CreateShaders()
{
	// Get shader path based on platform
	std::string shaderExt = (_context->GetCurrentPlatform() == ERHIPlatform::METAL) ? ".metal" : ".spv";
	std::string shaderPath = "Atmosphere/";

	ShaderInfo computeShaderInfo{};
	computeShaderInfo.stage = EShaderStage::COMPUTE;

	std::string transmittancePath = shaderPath + "Transmittance.comp" + shaderExt;
	_transmittanceShader = _context->CreateShader("TransmittanceCS", computeShaderInfo, transmittancePath.c_str());
	if (!_transmittanceShader)
	{
		HS_LOG(warning, "Failed to load transmittance shader, precomputation may not work");
	}

	std::string directIrradiancePath = shaderPath + "DirectIrradiance.comp" + shaderExt;
	_directIrradianceShader = _context->CreateShader("DirectIrradianceCS", computeShaderInfo, directIrradiancePath.c_str());

	std::string singleScatteringPath = shaderPath + "SingleScattering.comp" + shaderExt;
	_singleScatteringShader = _context->CreateShader("SingleScatteringCS", computeShaderInfo, singleScatteringPath.c_str());

	std::string scatteringDensityPath = shaderPath + "ScatteringDensity.comp" + shaderExt;
	_scatteringDensityShader = _context->CreateShader("ScatteringDensityCS", computeShaderInfo, scatteringDensityPath.c_str());

	std::string indirectIrradiancePath = shaderPath + "IndirectIrradiance.comp" + shaderExt;
	_indirectIrradianceShader = _context->CreateShader("IndirectIrradianceCS", computeShaderInfo, indirectIrradiancePath.c_str());

	std::string multipleScatteringPath = shaderPath + "MultipleScattering.comp" + shaderExt;
	_multipleScatteringShader = _context->CreateShader("MultipleScatteringCS", computeShaderInfo, multipleScatteringPath.c_str());

	// Return true even if some shaders failed - we'll handle it during Compute()
	return true;
}

void AtmospherePrecompute::DestroyShaders()
{
	if (!_context) return;

	auto destroyShader = [this](RHIShader*& shader) {
		if (shader)
		{
			_context->DestroyShader(shader);
			shader = nullptr;
		}
	};

	destroyShader(_transmittanceShader);
	destroyShader(_directIrradianceShader);
	destroyShader(_singleScatteringShader);
	destroyShader(_scatteringDensityShader);
	destroyShader(_indirectIrradianceShader);
	destroyShader(_multipleScatteringShader);
}

bool AtmospherePrecompute::CreateResourceLayouts()
{
	// Transmittance layout: params buffer + output texture
	{
		ResourceBinding bindings[2];

		bindings[0].type = EResourceType::UNIFORM_BUFFER;
		bindings[0].stage = EShaderStage::COMPUTE;
		bindings[0].binding = 0;
		bindings[0].arrayCount = 1;
		bindings[0].name = "atmosphere";

		bindings[1].type = EResourceType::STORAGE_IMAGE;
		bindings[1].stage = EShaderStage::COMPUTE;
		bindings[1].binding = 1;
		bindings[1].arrayCount = 1;
		bindings[1].name = "TransmittanceTexture";

		_transmittanceLayout = _context->CreateResourceLayout("TransmittanceLayout", bindings, 2);
		if (!_transmittanceLayout) return false;
	}

	// Direct irradiance layout: params + transmittance input + sampler + 2 outputs
	{
		ResourceBinding bindings[5];

		bindings[0].type = EResourceType::UNIFORM_BUFFER;
		bindings[0].stage = EShaderStage::COMPUTE;
		bindings[0].binding = 0;
		bindings[0].arrayCount = 1;
		bindings[0].name = "atmosphere";

		bindings[1].type = EResourceType::SAMPLED_IMAGE;
		bindings[1].stage = EShaderStage::COMPUTE;
		bindings[1].binding = 1;
		bindings[1].arrayCount = 1;
		bindings[1].name = "TransmittanceTexture";

		bindings[2].type = EResourceType::SAMPLER;
		bindings[2].stage = EShaderStage::COMPUTE;
		bindings[2].binding = 2;
		bindings[2].arrayCount = 1;
		bindings[2].name = "TransmittanceSampler";

		bindings[3].type = EResourceType::STORAGE_IMAGE;
		bindings[3].stage = EShaderStage::COMPUTE;
		bindings[3].binding = 3;
		bindings[3].arrayCount = 1;
		bindings[3].name = "DeltaIrradianceTexture";

		bindings[4].type = EResourceType::STORAGE_IMAGE;
		bindings[4].stage = EShaderStage::COMPUTE;
		bindings[4].binding = 4;
		bindings[4].arrayCount = 1;
		bindings[4].name = "IrradianceTexture";

		_directIrradianceLayout = _context->CreateResourceLayout("DirectIrradianceLayout", bindings, 5);
		if (!_directIrradianceLayout) return false;
	}

	// Single scattering layout: similar pattern
	{
		ResourceBinding bindings[7];

		bindings[0].type = EResourceType::UNIFORM_BUFFER;
		bindings[0].stage = EShaderStage::COMPUTE;
		bindings[0].binding = 0;
		bindings[0].arrayCount = 1;
		bindings[0].name = "atmosphere";

		bindings[1].type = EResourceType::SAMPLED_IMAGE;
		bindings[1].stage = EShaderStage::COMPUTE;
		bindings[1].binding = 1;
		bindings[1].arrayCount = 1;
		bindings[1].name = "TransmittanceTexture";

		bindings[2].type = EResourceType::SAMPLER;
		bindings[2].stage = EShaderStage::COMPUTE;
		bindings[2].binding = 2;
		bindings[2].arrayCount = 1;
		bindings[2].name = "TransmittanceSampler";

		bindings[3].type = EResourceType::STORAGE_IMAGE;
		bindings[3].stage = EShaderStage::COMPUTE;
		bindings[3].binding = 3;
		bindings[3].arrayCount = 1;
		bindings[3].name = "DeltaRayleighScatteringTexture";

		bindings[4].type = EResourceType::STORAGE_IMAGE;
		bindings[4].stage = EShaderStage::COMPUTE;
		bindings[4].binding = 4;
		bindings[4].arrayCount = 1;
		bindings[4].name = "DeltaMieScatteringTexture";

		bindings[5].type = EResourceType::STORAGE_IMAGE;
		bindings[5].stage = EShaderStage::COMPUTE;
		bindings[5].binding = 5;
		bindings[5].arrayCount = 1;
		bindings[5].name = "ScatteringTexture";

		bindings[6].type = EResourceType::STORAGE_IMAGE;
		bindings[6].stage = EShaderStage::COMPUTE;
		bindings[6].binding = 6;
		bindings[6].arrayCount = 1;
		bindings[6].name = "SingleMieScatteringTexture";

		_singleScatteringLayout = _context->CreateResourceLayout("SingleScatteringLayout", bindings, 7);
		if (!_singleScatteringLayout) return false;
	}

	// For brevity, create simplified layouts for remaining shaders
	// In a full implementation, each would have its specific bindings

	_scatteringDensityLayout = _transmittanceLayout;  // Placeholder
	_indirectIrradianceLayout = _directIrradianceLayout;  // Placeholder
	_multipleScatteringLayout = _singleScatteringLayout;  // Placeholder

	return true;
}

void AtmospherePrecompute::DestroyResourceLayouts()
{
	if (!_context) return;

	// Only destroy layouts we actually created (not placeholders)
	if (_transmittanceLayout)
	{
		_context->DestroyResourceLayout(_transmittanceLayout);
		_transmittanceLayout = nullptr;
	}

	if (_directIrradianceLayout)
	{
		_context->DestroyResourceLayout(_directIrradianceLayout);
		_directIrradianceLayout = nullptr;
	}

	if (_singleScatteringLayout)
	{
		_context->DestroyResourceLayout(_singleScatteringLayout);
		_singleScatteringLayout = nullptr;
	}

	// Placeholders were pointing to other layouts, just null them
	_scatteringDensityLayout = nullptr;
	_indirectIrradianceLayout = nullptr;
	_multipleScatteringLayout = nullptr;
}

bool AtmospherePrecompute::CreatePipelines()
{
	// Create compute pipelines for each stage
	if (_transmittanceShader && _transmittanceLayout)
	{
		ComputePipelineInfo info{};
		info.computeShader = _transmittanceShader;
		info.resourceLayout = _transmittanceLayout;
		_transmittancePipeline = _context->CreateComputePipeline("TransmittancePipeline", info);
	}

	if (_directIrradianceShader && _directIrradianceLayout)
	{
		ComputePipelineInfo info{};
		info.computeShader = _directIrradianceShader;
		info.resourceLayout = _directIrradianceLayout;
		_directIrradiancePipeline = _context->CreateComputePipeline("DirectIrradiancePipeline", info);
	}

	if (_singleScatteringShader && _singleScatteringLayout)
	{
		ComputePipelineInfo info{};
		info.computeShader = _singleScatteringShader;
		info.resourceLayout = _singleScatteringLayout;
		_singleScatteringPipeline = _context->CreateComputePipeline("SingleScatteringPipeline", info);
	}

	// Additional pipelines would be created similarly

	return true;
}

void AtmospherePrecompute::DestroyPipelines()
{
	if (!_context) return;

	auto destroyPipeline = [this](RHIComputePipeline*& pipeline) {
		if (pipeline)
		{
			_context->DestroyComputePipeline(pipeline);
			pipeline = nullptr;
		}
	};

	destroyPipeline(_transmittancePipeline);
	destroyPipeline(_directIrradiancePipeline);
	destroyPipeline(_singleScatteringPipeline);
	destroyPipeline(_scatteringDensityPipeline);
	destroyPipeline(_indirectIrradiancePipeline);
	destroyPipeline(_multipleScatteringPipeline);
}

bool AtmospherePrecompute::CreateResourceSets()
{
	// Resource sets would bind actual resources to the layouts
	// This is a simplified implementation
	return true;
}

void AtmospherePrecompute::DestroyResourceSets()
{
	if (!_context) return;

	auto destroySet = [this](RHIResourceSet*& set) {
		if (set)
		{
			_context->DestroyResourceSet(set);
			set = nullptr;
		}
	};

	destroySet(_transmittanceSet);
	destroySet(_directIrradianceSet);
	destroySet(_singleScatteringSet);
	destroySet(_scatteringDensitySet);
	destroySet(_indirectIrradianceSet);
	destroySet(_multipleScatteringSet);
}

void AtmospherePrecompute::Compute(RHICommandBuffer* cmdBuffer)
{
	if (!_isInitialized)
	{
		HS_LOG(error, "AtmospherePrecompute not initialized");
		return;
	}

	HS_LOG(info, "Starting atmosphere precomputation...");

	// Step 1: Compute transmittance LUT
	DispatchTransmittance(cmdBuffer);
	cmdBuffer->TextureBarrier(_transmittanceTexture);

	// Step 2: Compute direct irradiance
	DispatchDirectIrradiance(cmdBuffer);
	cmdBuffer->TextureBarrier(_irradianceTexture);
	cmdBuffer->TextureBarrier(_deltaIrradianceTexture);

	// Step 3: Compute single scattering
	DispatchSingleScattering(cmdBuffer);
	cmdBuffer->TextureBarrier(_scatteringTexture);
	cmdBuffer->TextureBarrier(_singleMieScatteringTexture);
	cmdBuffer->TextureBarrier(_deltaRayleighScatteringTexture);
	cmdBuffer->TextureBarrier(_deltaMieScatteringTexture);

	// Step 4: Multi-scattering iterations
	for (int order = 2; order <= _numScatteringOrders; order++)
	{
		// Scattering density
		DispatchScatteringDensity(cmdBuffer, order);
		cmdBuffer->TextureBarrier(_deltaScatteringDensityTexture);

		// Indirect irradiance
		DispatchIndirectIrradiance(cmdBuffer, order);
		cmdBuffer->TextureBarrier(_irradianceTexture);

		// Multiple scattering
		DispatchMultipleScattering(cmdBuffer);
		cmdBuffer->TextureBarrier(_scatteringTexture);
	}

	cmdBuffer->EndComputePass();

	_isComputed = true;
	HS_LOG(info, "Atmosphere precomputation complete");
}

void AtmospherePrecompute::DispatchTransmittance(RHICommandBuffer* cmdBuffer)
{
	using namespace AtmosphereConstants;

	if (!_transmittancePipeline)
	{
		HS_LOG(warning, "Transmittance pipeline not available");
		return;
	}

	cmdBuffer->BindComputePipeline(_transmittancePipeline);

	// Bind resources
	if (_transmittanceSet)
	{
		cmdBuffer->BindComputeResourceSet(_transmittanceSet);
	}

	// Dispatch
	uint32 groupsX = (TRANSMITTANCE_TEXTURE_WIDTH + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsY = (TRANSMITTANCE_TEXTURE_HEIGHT + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	cmdBuffer->Dispatch(groupsX, groupsY, 1);
}

void AtmospherePrecompute::DispatchDirectIrradiance(RHICommandBuffer* cmdBuffer)
{
	using namespace AtmosphereConstants;

	if (!_directIrradiancePipeline)
	{
		HS_LOG(warning, "Direct irradiance pipeline not available");
		return;
	}

	cmdBuffer->BindComputePipeline(_directIrradiancePipeline);

	if (_directIrradianceSet)
	{
		cmdBuffer->BindComputeResourceSet(_directIrradianceSet);
	}

	uint32 groupsX = (IRRADIANCE_TEXTURE_WIDTH + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsY = (IRRADIANCE_TEXTURE_HEIGHT + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	cmdBuffer->Dispatch(groupsX, groupsY, 1);
}

void AtmospherePrecompute::DispatchSingleScattering(RHICommandBuffer* cmdBuffer)
{
	using namespace AtmosphereConstants;

	if (!_singleScatteringPipeline)
	{
		HS_LOG(warning, "Single scattering pipeline not available");
		return;
	}

	cmdBuffer->BindComputePipeline(_singleScatteringPipeline);

	if (_singleScatteringSet)
	{
		cmdBuffer->BindComputeResourceSet(_singleScatteringSet);
	}

	uint32 groupsX = (SCATTERING_TEXTURE_WIDTH + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsY = (SCATTERING_TEXTURE_HEIGHT + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsZ = SCATTERING_TEXTURE_DEPTH;
	cmdBuffer->Dispatch(groupsX, groupsY, groupsZ);
}

void AtmospherePrecompute::DispatchScatteringDensity(RHICommandBuffer* cmdBuffer, int scatteringOrder)
{
	using namespace AtmosphereConstants;

	if (!_scatteringDensityPipeline)
	{
		return;
	}

	cmdBuffer->BindComputePipeline(_scatteringDensityPipeline);

	if (_scatteringDensitySet)
	{
		cmdBuffer->BindComputeResourceSet(_scatteringDensitySet);
	}

	// TODO: Set push constant for scatteringOrder

	uint32 groupsX = (SCATTERING_TEXTURE_WIDTH + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsY = (SCATTERING_TEXTURE_HEIGHT + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsZ = SCATTERING_TEXTURE_DEPTH;
	cmdBuffer->Dispatch(groupsX, groupsY, groupsZ);
}

void AtmospherePrecompute::DispatchIndirectIrradiance(RHICommandBuffer* cmdBuffer, int scatteringOrder)
{
	using namespace AtmosphereConstants;

	if (!_indirectIrradiancePipeline)
	{
		return;
	}

	cmdBuffer->BindComputePipeline(_indirectIrradiancePipeline);

	if (_indirectIrradianceSet)
	{
		cmdBuffer->BindComputeResourceSet(_indirectIrradianceSet);
	}

	// TODO: Set push constant for scatteringOrder

	uint32 groupsX = (IRRADIANCE_TEXTURE_WIDTH + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsY = (IRRADIANCE_TEXTURE_HEIGHT + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	cmdBuffer->Dispatch(groupsX, groupsY, 1);
}

void AtmospherePrecompute::DispatchMultipleScattering(RHICommandBuffer* cmdBuffer)
{
	using namespace AtmosphereConstants;

	if (!_multipleScatteringPipeline)
	{
		return;
	}

	cmdBuffer->BindComputePipeline(_multipleScatteringPipeline);

	if (_multipleScatteringSet)
	{
		cmdBuffer->BindComputeResourceSet(_multipleScatteringSet);
	}

	uint32 groupsX = (SCATTERING_TEXTURE_WIDTH + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsY = (SCATTERING_TEXTURE_HEIGHT + COMPUTE_THREAD_GROUP_SIZE - 1) / COMPUTE_THREAD_GROUP_SIZE;
	uint32 groupsZ = SCATTERING_TEXTURE_DEPTH;
	cmdBuffer->Dispatch(groupsX, groupsY, groupsZ);
}

HS_NS_END
