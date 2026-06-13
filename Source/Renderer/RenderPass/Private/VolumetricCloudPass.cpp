#include "Renderer/RenderPass/VolumetricCloudPass.h"

#include "Core/Hash.h"
#include "Core/Log.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"

#include "Renderer/ShaderLibrary.h"
#include "Resource/Shader.h"

#include <algorithm>
#include <cmath>

HS_NS_BEGIN

namespace
{
constexpr uint32 BaseNoiseSize = 128;
constexpr uint32 DetailNoiseSize = 32;
constexpr uint32 CurlWeatherSize = 128;

float saturate(float value)
{
    return std::clamp(value, 0.0f, 1.0f);
}

uint32 hashU32(uint32 x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

float hash01(int x, int y, int z, uint32 seed)
{
    uint32 h = static_cast<uint32>(x) * 73856093u ^
               static_cast<uint32>(y) * 19349663u ^
               static_cast<uint32>(z) * 83492791u ^
               seed * 2654435761u;
    return static_cast<float>(hashU32(h) & 0x00ffffffu) / static_cast<float>(0x01000000u);
}

float smooth(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float valueNoise3D(float x, float y, float z, int frequency, uint32 seed)
{
    x = x * static_cast<float>(frequency);
    y = y * static_cast<float>(frequency);
    z = z * static_cast<float>(frequency);

    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    int iz = static_cast<int>(std::floor(z));
    float fx = smooth(x - static_cast<float>(ix));
    float fy = smooth(y - static_cast<float>(iy));
    float fz = smooth(z - static_cast<float>(iz));

    auto sample = [&](int ox, int oy, int oz) -> float
    {
        int sx = (ix + ox) % frequency; if (sx < 0) sx += frequency;
        int sy = (iy + oy) % frequency; if (sy < 0) sy += frequency;
        int sz = (iz + oz) % frequency; if (sz < 0) sz += frequency;
        return hash01(sx, sy, sz, seed);
    };

    float x00 = lerp(sample(0, 0, 0), sample(1, 0, 0), fx);
    float x10 = lerp(sample(0, 1, 0), sample(1, 1, 0), fx);
    float x01 = lerp(sample(0, 0, 1), sample(1, 0, 1), fx);
    float x11 = lerp(sample(0, 1, 1), sample(1, 1, 1), fx);
    float y0 = lerp(x00, x10, fy);
    float y1 = lerp(x01, x11, fy);
    return lerp(y0, y1, fz);
}

float fbm3D(float x, float y, float z, int baseFrequency, int octaves, uint32 seed)
{
    float value = 0.0f;
    float amplitude = 0.5f;
    float total = 0.0f;
    int frequency = baseFrequency;
    for (int i = 0; i < octaves; ++i)
    {
        value += valueNoise3D(x, y, z, frequency, seed + static_cast<uint32>(i) * 17u) * amplitude;
        total += amplitude;
        amplitude *= 0.5f;
        frequency *= 2;
    }
    return total > 0.0f ? value / total : 0.0f;
}

float worley3D(float x, float y, float z, int frequency, uint32 seed)
{
    float px = x * static_cast<float>(frequency);
    float py = y * static_cast<float>(frequency);
    float pz = z * static_cast<float>(frequency);
    int cx = static_cast<int>(std::floor(px));
    int cy = static_cast<int>(std::floor(py));
    int cz = static_cast<int>(std::floor(pz));
    float best = 999.0f;

    for (int dz = -1; dz <= 1; ++dz)
    for (int dy = -1; dy <= 1; ++dy)
    for (int dx = -1; dx <= 1; ++dx)
    {
        int gx = (cx + dx) % frequency; if (gx < 0) gx += frequency;
        int gy = (cy + dy) % frequency; if (gy < 0) gy += frequency;
        int gz = (cz + dz) % frequency; if (gz < 0) gz += frequency;

        float fx = (static_cast<float>(gx) + hash01(gx, gy, gz, seed + 1u)) / static_cast<float>(frequency);
        float fy = (static_cast<float>(gy) + hash01(gx, gy, gz, seed + 2u)) / static_cast<float>(frequency);
        float fz = (static_cast<float>(gz) + hash01(gx, gy, gz, seed + 3u)) / static_cast<float>(frequency);

        float vx = std::abs(x - fx); vx = std::min(vx, 1.0f - vx);
        float vy = std::abs(y - fy); vy = std::min(vy, 1.0f - vy);
        float vz = std::abs(z - fz); vz = std::min(vz, 1.0f - vz);
        best = std::min(best, std::sqrt(vx * vx + vy * vy + vz * vz) * static_cast<float>(frequency));
    }

    return 1.0f - saturate(best / 1.7320508f);
}

float valueNoise2D(float x, float y, int frequency, uint32 seed)
{
    return valueNoise3D(x, y, 0.37f, frequency, seed);
}

uint8 toByte(float value)
{
    return static_cast<uint8>(std::round(saturate(value) * 255.0f));
}
}

VolumetricCloudPass::~VolumetricCloudPass()
{
    Shutdown();
}

bool VolumetricCloudPass::Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext)
{
    _rhiContext = rhiContext;

    Shader* shader = shaderLibrary->GetOrCompile("VolumetricCloud", EShaderStage::Vertex | EShaderStage::Fragment);
    if (!shader || !shader->IsCompiledEx())
    {
        HS_LOG(error, "[VolumetricCloudPass] VolumetricCloud shader compilation failed");
        return false;
    }

    const auto* vsBytecode = shader->GetByteCode(EShaderStage::Vertex);
    const auto* fsBytecode = shader->GetByteCode(EShaderStage::Fragment);
    if (!vsBytecode || !fsBytecode)
    {
        HS_LOG(error, "[VolumetricCloudPass] Shader bytecode not found");
        return false;
    }

    ShaderInfo vsInfo{};
    vsInfo.stage = EShaderStage::Vertex;
    vsInfo.entryName = shader->GetEntryPoint(EShaderStage::Vertex);
    _vertexShader = _rhiContext->CreateShader(
        "VolumetricCloudVS", vsInfo,
        reinterpret_cast<const char*>(vsBytecode->data()),
        vsBytecode->size());

    ShaderInfo fsInfo{};
    fsInfo.stage = EShaderStage::Fragment;
    fsInfo.entryName = shader->GetEntryPoint(EShaderStage::Fragment);
    _fragmentShader = _rhiContext->CreateShader(
        "VolumetricCloudFS", fsInfo,
        reinterpret_cast<const char*>(fsBytecode->data()),
        fsBytecode->size());

    if (!_vertexShader || !_fragmentShader)
    {
        HS_LOG(error, "[VolumetricCloudPass] Failed to create RHI shaders");
        return false;
    }

    CloudSettingsUBO defaultSettings{};
    _settingsBuffer = _rhiContext->CreateBuffer(
        "VolumetricCloudSettings",
        &defaultSettings,
        sizeof(defaultSettings),
        EBufferUsage::Uniform,
        EBufferMemoryOption::Dynamic);

    if (!_settingsBuffer || !createNoiseResources())
    {
        HS_LOG(error, "[VolumetricCloudPass] Failed to create GPU resources");
        return false;
    }

    _isInitialized = true;
    return true;
}

void VolumetricCloudPass::Shutdown()
{
    if (!_rhiContext) return;

    _rhiContext->WaitForIdle();
    invalidatePipelines();
    destroyGpuResources();

    if (_fragmentShader)
    {
        _rhiContext->DestroyShader(_fragmentShader);
        _fragmentShader = nullptr;
    }
    if (_vertexShader)
    {
        _rhiContext->DestroyShader(_vertexShader);
        _vertexShader = nullptr;
    }

    _perViewBuffer = nullptr;
    _isInitialized = false;
    _rhiContext = nullptr;
}

void VolumetricCloudPass::UpdateSettings(const VolumetricCloudSettings& settings, float timeSeconds)
{
    if (!_settingsBuffer) return;

    glm::vec2 wind = glm::length(settings.windDirection) > 0.0001f
        ? glm::normalize(settings.windDirection)
        : glm::vec2(1.0f, 0.0f);
    glm::vec3 sun = glm::length(settings.sunDirection) > 0.0001f
        ? glm::normalize(settings.sunDirection)
        : glm::vec3(0.0f, 1.0f, 0.0f);

    CloudSettingsUBO ubo{};
    ubo.sunDirectionIntensity = glm::vec4(sun, settings.sunIntensity);
    ubo.sunColorAmbient = glm::vec4(settings.sunColor, settings.ambientIntensity);
    ubo.coverageTypePrecipDensity = glm::vec4(
        settings.coverage,
        settings.cloudType,
        settings.precipitation,
        settings.densityMultiplier);
    ubo.erosionWindDebugTime = glm::vec4(
        settings.erosion,
        wind.x * settings.windSpeed,
        wind.y * settings.windSpeed,
        timeSeconds);
    ubo.layerAndAtmosphere = glm::vec4(
        settings.cloudBottomMeters,
        settings.cloudTopMeters,
        6360000.0f,
        static_cast<float>(settings.debugView));
    ubo.sampleAndLighting = glm::vec4(
        static_cast<float>(std::clamp(settings.primarySampleCount, 16u, 160u)),
        static_cast<float>(std::clamp(settings.lightSampleCount, 1u, 8u)),
        settings.hgG,
        settings.powderStrength);

    _rhiContext->UpdateBuffer(_settingsBuffer, 0, &ubo, sizeof(ubo));
}

bool VolumetricCloudPass::createNoiseResources()
{
    std::vector<uint8> baseNoise = buildBaseNoise(BaseNoiseSize);
    TextureInfo baseInfo{};
    baseInfo.type = ETextureType::Tex3D;
    baseInfo.format = EPixelFormat::R8G8B8A8Unorm;
    baseInfo.usage = ETextureUsage::Sampled | ETextureUsage::TransferDestination;
    baseInfo.extent.width = BaseNoiseSize;
    baseInfo.extent.height = BaseNoiseSize;
    baseInfo.extent.depth = BaseNoiseSize;
    baseInfo.byteSize = baseNoise.size();
    _baseNoise = _rhiContext->CreateTexture("CloudBaseNoise", baseNoise.data(), baseInfo);

    std::vector<uint8> detailNoise = buildDetailNoise(DetailNoiseSize);
    TextureInfo detailInfo{};
    detailInfo.type = ETextureType::Tex3D;
    detailInfo.format = EPixelFormat::R8G8B8A8Unorm;
    detailInfo.usage = ETextureUsage::Sampled | ETextureUsage::TransferDestination;
    detailInfo.extent.width = DetailNoiseSize;
    detailInfo.extent.height = DetailNoiseSize;
    detailInfo.extent.depth = DetailNoiseSize;
    detailInfo.byteSize = detailNoise.size();
    _detailNoise = _rhiContext->CreateTexture("CloudDetailNoise", detailNoise.data(), detailInfo);

    std::vector<uint8> curlWeather = buildCurlWeatherTexture(CurlWeatherSize);
    TextureInfo curlInfo{};
    curlInfo.type = ETextureType::Tex2D;
    curlInfo.format = EPixelFormat::R8G8B8A8Unorm;
    curlInfo.usage = ETextureUsage::Sampled | ETextureUsage::TransferDestination;
    curlInfo.extent.width = CurlWeatherSize;
    curlInfo.extent.height = CurlWeatherSize;
    curlInfo.extent.depth = 1;
    curlInfo.byteSize = curlWeather.size();
    _curlWeather = _rhiContext->CreateTexture("CloudCurlWeather", curlWeather.data(), curlInfo);

    SamplerInfo volumeSampler{};
    volumeSampler.type = ETextureType::Tex3D;
    volumeSampler.minFilter = EFilterMode::Linear;
    volumeSampler.magFilter = EFilterMode::Linear;
    volumeSampler.mipmapMode = EFilterMode::Linear;
    volumeSampler.addressU = EAddressMode::Repeat;
    volumeSampler.addressV = EAddressMode::Repeat;
    volumeSampler.addressW = EAddressMode::Repeat;
    _volumeSampler = _rhiContext->CreateSampler("CloudVolumeSampler", volumeSampler);

    SamplerInfo weatherSampler{};
    weatherSampler.type = ETextureType::Tex2D;
    weatherSampler.minFilter = EFilterMode::Linear;
    weatherSampler.magFilter = EFilterMode::Linear;
    weatherSampler.mipmapMode = EFilterMode::Linear;
    weatherSampler.addressU = EAddressMode::Repeat;
    weatherSampler.addressV = EAddressMode::Repeat;
    weatherSampler.addressW = EAddressMode::Repeat;
    _weatherSampler = _rhiContext->CreateSampler("CloudWeatherSampler", weatherSampler);

    return _baseNoise && _detailNoise && _curlWeather && _volumeSampler && _weatherSampler;
}

void VolumetricCloudPass::destroyGpuResources()
{
    if (_resourceSet)
    {
        _rhiContext->DestroyResourceSet(_resourceSet);
        _resourceSet = nullptr;
    }
    if (_resourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_resourceLayout);
        _resourceLayout = nullptr;
    }
    if (_weatherSampler)
    {
        _rhiContext->DestroySampler(_weatherSampler);
        _weatherSampler = nullptr;
    }
    if (_volumeSampler)
    {
        _rhiContext->DestroySampler(_volumeSampler);
        _volumeSampler = nullptr;
    }
    if (_curlWeather)
    {
        _rhiContext->DestroyTexture(_curlWeather);
        _curlWeather = nullptr;
    }
    if (_detailNoise)
    {
        _rhiContext->DestroyTexture(_detailNoise);
        _detailNoise = nullptr;
    }
    if (_baseNoise)
    {
        _rhiContext->DestroyTexture(_baseNoise);
        _baseNoise = nullptr;
    }
    if (_settingsBuffer)
    {
        _rhiContext->DestroyBuffer(_settingsBuffer);
        _settingsBuffer = nullptr;
    }
}

void VolumetricCloudPass::invalidatePipelines()
{
    for (auto& [key, pipeline] : _pipelineCache)
    {
        if (pipeline)
        {
            _rhiContext->DestroyGraphicsPipeline(pipeline);
        }
    }
    _pipelineCache.clear();
}

void VolumetricCloudPass::rebuildResourceBindings(RHIBuffer* perViewBuffer)
{
    if (_resourceSet)
    {
        _rhiContext->DestroyResourceSet(_resourceSet);
        _resourceSet = nullptr;
    }
    if (_resourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_resourceLayout);
        _resourceLayout = nullptr;
    }

    ResourceBinding perViewBinding{};
    perViewBinding.type = EResourceType::UniformBuffer;
    perViewBinding.stage = EShaderStage::Vertex | EShaderStage::Fragment;
    perViewBinding.binding = 0;
    perViewBinding.arrayCount = 1;
    perViewBinding.resource.buffers.push_back(perViewBuffer);
    perViewBinding.resource.offsets.push_back(0);

    ResourceBinding settingsBinding{};
    settingsBinding.type = EResourceType::UniformBuffer;
    settingsBinding.stage = EShaderStage::Fragment;
    settingsBinding.binding = 1;
    settingsBinding.arrayCount = 1;
    settingsBinding.resource.buffers.push_back(_settingsBuffer);
    settingsBinding.resource.offsets.push_back(0);

    ResourceBinding baseBinding{};
    baseBinding.type = EResourceType::CombinedImageSampler;
    baseBinding.stage = EShaderStage::Fragment;
    baseBinding.binding = 2;
    baseBinding.arrayCount = 1;
    baseBinding.resource.textures.push_back(_baseNoise);
    baseBinding.resource.samplers.push_back(_volumeSampler);

    ResourceBinding detailBinding = baseBinding;
    detailBinding.binding = 3;
    detailBinding.resource.textures.clear();
    detailBinding.resource.samplers.clear();
    detailBinding.resource.textures.push_back(_detailNoise);
    detailBinding.resource.samplers.push_back(_volumeSampler);

    ResourceBinding weatherBinding{};
    weatherBinding.type = EResourceType::CombinedImageSampler;
    weatherBinding.stage = EShaderStage::Fragment;
    weatherBinding.binding = 4;
    weatherBinding.arrayCount = 1;
    weatherBinding.resource.textures.push_back(_curlWeather);
    weatherBinding.resource.samplers.push_back(_weatherSampler);

    ResourceBinding bindings[5] = { perViewBinding, settingsBinding, baseBinding, detailBinding, weatherBinding };
    _resourceLayout = _rhiContext->CreateResourceLayout("VolumetricCloudLayout", bindings, 5);
    if (!_resourceLayout)
    {
        HS_LOG(error, "[VolumetricCloudPass] Failed to create ResourceLayout");
        return;
    }

    _resourceSet = _rhiContext->CreateResourceSet("VolumetricCloudSet", _resourceLayout);
    if (!_resourceSet)
    {
        HS_LOG(error, "[VolumetricCloudPass] Failed to create ResourceSet");
        return;
    }

    _perViewBuffer = perViewBuffer;
}

RHIGraphicsPipeline* VolumetricCloudPass::GetOrCreatePipeline(
    const PipelineRenderTargetLayout& renderTargetLayout,
    RHIBuffer* perViewBuffer)
{
    if (!_isInitialized || !perViewBuffer) return nullptr;

    if (perViewBuffer != _perViewBuffer)
    {
        rebuildResourceBindings(perViewBuffer);
        invalidatePipelines();
    }

    if (!_resourceLayout || !_resourceSet) return nullptr;

    size_t pipelineKey = std::hash<PipelineRenderTargetLayout>{}(renderTargetLayout);
    auto it = _pipelineCache.find(pipelineKey);
    if (it != _pipelineCache.end())
    {
        return it->second;
    }

    VertexInputStateDescriptor viDesc{};

    InputAssemblyStateDescriptor iaDesc{};
    iaDesc.primitiveTopology = EPrimitiveTopology::TriangleList;
    iaDesc.isRestartEnable = false;

    RasterizerStateDescriptor rsDesc{};
    rsDesc.cullMode = ECullMode::None;
    rsDesc.frontFace = EFrontFace::CounterClockwise;
    rsDesc.polygonMode = EPolygonMode::Fill;
    rsDesc.depthClampEnable = false;
    rsDesc.rasterizerDiscardEnable = false;
    rsDesc.depthBiasEnable = false;
    rsDesc.lineWidth = 1.0f;

    DepthStencilStateDescriptor dsDesc{};
    dsDesc.depthTestEnable = renderTargetLayout.useDepthStencilAttachment;
    dsDesc.depthWriteEnable = false;
    dsDesc.depthCompareOp = ECompareOp::LessOrEqual;
    dsDesc.depthBoundTestEnable = false;
    dsDesc.stencilTestEnable = false;

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.logicOpEnable = false;
    cbDesc.attachmentCount = renderTargetLayout.colorAttachmentCount;
    cbDesc.attachments.resize(cbDesc.attachmentCount);
    for (ColorBlendAttachmentDescriptor& att : cbDesc.attachments)
    {
        att.blendEnable = false;
        att.srcColorFactor = EBlendFactor::One;
        att.dstColorFactor = EBlendFactor::Zero;
        att.colorBlendOp = EBlendOp::Add;
        att.srcAlphaFactor = EBlendFactor::One;
        att.dstAlphaFactor = EBlendFactor::Zero;
        att.alphaBlendOp = EBlendOp::Add;
    }

    ShaderProgramDescriptor spDesc{};
    spDesc.stages.push_back(_vertexShader);
    spDesc.stages.push_back(_fragmentShader);

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc = spDesc;
    gpInfo.inputAssemblyDesc = iaDesc;
    gpInfo.vertexInputDesc = viDesc;
    gpInfo.rasterizerDesc = rsDesc;
    gpInfo.depthStencilDesc = dsDesc;
    gpInfo.colorBlendDesc = cbDesc;
    gpInfo.renderTargetLayout = renderTargetLayout;
    gpInfo.resourceLayout = _resourceLayout;

    RHIGraphicsPipeline* pipeline = _rhiContext->CreateGraphicsPipeline("VolumetricCloudPipeline", gpInfo);
    if (pipeline)
    {
        _pipelineCache[pipelineKey] = pipeline;
    }
    else
    {
        HS_LOG(error, "[VolumetricCloudPass] Failed to create GraphicsPipeline");
    }

    return pipeline;
}

std::vector<uint8> VolumetricCloudPass::buildBaseNoise(uint32 size)
{
    std::vector<uint8> data(static_cast<size_t>(size) * size * size * 4);
    for (uint32 z = 0; z < size; ++z)
    for (uint32 y = 0; y < size; ++y)
    for (uint32 x = 0; x < size; ++x)
    {
        float px = static_cast<float>(x) / static_cast<float>(size);
        float py = static_cast<float>(y) / static_cast<float>(size);
        float pz = static_cast<float>(z) / static_cast<float>(size);

        float perlinLike = fbm3D(px, py, pz, 4, 5, 11u);
        float worleyLow = worley3D(px, py, pz, 4, 31u);
        float worleyMid = worley3D(px, py, pz, 8, 47u);
        float worleyHigh = worley3D(px, py, pz, 16, 59u);
        float perlinWorley = saturate(lerp(perlinLike, worleyLow, 0.38f));

        size_t idx = (static_cast<size_t>(z) * size * size + static_cast<size_t>(y) * size + x) * 4;
        data[idx + 0] = toByte(perlinWorley);
        data[idx + 1] = toByte(worleyLow);
        data[idx + 2] = toByte(worleyMid);
        data[idx + 3] = toByte(worleyHigh);
    }
    return data;
}

std::vector<uint8> VolumetricCloudPass::buildDetailNoise(uint32 size)
{
    std::vector<uint8> data(static_cast<size_t>(size) * size * size * 4);
    for (uint32 z = 0; z < size; ++z)
    for (uint32 y = 0; y < size; ++y)
    for (uint32 x = 0; x < size; ++x)
    {
        float px = static_cast<float>(x) / static_cast<float>(size);
        float py = static_cast<float>(y) / static_cast<float>(size);
        float pz = static_cast<float>(z) / static_cast<float>(size);

        size_t idx = (static_cast<size_t>(z) * size * size + static_cast<size_t>(y) * size + x) * 4;
        data[idx + 0] = toByte(worley3D(px, py, pz, 8, 103u));
        data[idx + 1] = toByte(worley3D(px, py, pz, 16, 127u));
        data[idx + 2] = toByte(worley3D(px, py, pz, 24, 151u));
        data[idx + 3] = 255;
    }
    return data;
}

std::vector<uint8> VolumetricCloudPass::buildCurlWeatherTexture(uint32 size)
{
    std::vector<uint8> data(static_cast<size_t>(size) * size * 4);
    const float eps = 1.0f / static_cast<float>(size);
    for (uint32 y = 0; y < size; ++y)
    for (uint32 x = 0; x < size; ++x)
    {
        float u = static_cast<float>(x) / static_cast<float>(size);
        float v = static_cast<float>(y) / static_cast<float>(size);

        float nL = valueNoise2D(u - eps, v, 8, 211u);
        float nR = valueNoise2D(u + eps, v, 8, 211u);
        float nD = valueNoise2D(u, v - eps, 8, 211u);
        float nU = valueNoise2D(u, v + eps, 8, 211u);
        float curlX = (nU - nD) * 7.0f;
        float curlY = -(nR - nL) * 7.0f;
        float weather = fbm3D(u, v, 0.19f, 3, 4, 239u);

        size_t idx = (static_cast<size_t>(y) * size + x) * 4;
        data[idx + 0] = toByte(curlX * 0.5f + 0.5f);
        data[idx + 1] = toByte(curlY * 0.5f + 0.5f);
        data[idx + 2] = toByte(weather);
        data[idx + 3] = 255;
    }
    return data;
}

HS_NS_END
