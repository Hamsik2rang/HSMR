//
// Sky.frag.slang
// Fragment shader for atmospheric sky rendering
//
// Uses precomputed LUT textures to render physically-based atmosphere
//

#include "AtmosphereAPI.hlsli"

// =============================================================================
// Uniform Buffer
// =============================================================================

struct SkyUniforms
{
    float4x4 inverseViewProjection;
    float3   cameraPosition;
    float    _pad0;
    float3   sunDirection;
    float    sunAngularRadius;
    float3   earthCenter;
    float    exposure;
    float3   whitePoint;
    float    _pad1;
};

[[vk::binding(0, 0)]]
ConstantBuffer<SkyUniforms> uniforms;

// =============================================================================
// LUT Textures
// =============================================================================

[[vk::binding(1, 0)]]
Texture2D<float4> TransmittanceTexture;

[[vk::binding(2, 0)]]
Texture3D<float4> ScatteringTexture;

[[vk::binding(3, 0)]]
Texture2D<float4> IrradianceTexture;

[[vk::binding(4, 0)]]
Texture3D<float4> SingleMieScatteringTexture;

[[vk::binding(5, 0)]]
SamplerState LUTSampler;

// =============================================================================
// Input/Output
// =============================================================================

struct FragmentInput
{
    float4 position : SV_Position;
    float3 viewRay  : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

struct FragmentOutput
{
    float4 color : SV_Target0;
};

// =============================================================================
// Helper Functions
// =============================================================================

// Create atmosphere parameters from constants
AtmosphereParametersGPU GetAtmosphereParams()
{
    AtmosphereParametersGPU atmosphere;

    // Solar parameters
    atmosphere.solarIrradiance = float3(1.474, 1.850, 1.911);
    atmosphere.sunAngularRadius = uniforms.sunAngularRadius;

    // Planet parameters
    atmosphere.bottomRadius = 6360000.0;
    atmosphere.topRadius = 6420000.0;

    // Rayleigh scattering (layer[1] contains actual profile, layer[0].width=0 means use layer[1])
    atmosphere.rayleighDensity[0].width = 0.0;
    atmosphere.rayleighDensity[0].expTerm = 0.0;
    atmosphere.rayleighDensity[0].expScale = 0.0;
    atmosphere.rayleighDensity[0].linearTerm = 0.0;
    atmosphere.rayleighDensity[0].constantTerm = 0.0;
    atmosphere.rayleighDensity[1].width = 0.0;
    atmosphere.rayleighDensity[1].expTerm = 1.0;
    atmosphere.rayleighDensity[1].expScale = -1.0 / 8000.0;
    atmosphere.rayleighDensity[1].linearTerm = 0.0;
    atmosphere.rayleighDensity[1].constantTerm = 0.0;
    atmosphere.rayleighScattering = float3(5.802e-6, 13.558e-6, 33.1e-6);

    // Mie scattering (same pattern: layer[1] contains actual profile)
    atmosphere.mieDensity[0].width = 0.0;
    atmosphere.mieDensity[0].expTerm = 0.0;
    atmosphere.mieDensity[0].expScale = 0.0;
    atmosphere.mieDensity[0].linearTerm = 0.0;
    atmosphere.mieDensity[0].constantTerm = 0.0;
    atmosphere.mieDensity[1].width = 0.0;
    atmosphere.mieDensity[1].expTerm = 1.0;
    atmosphere.mieDensity[1].expScale = -1.0 / 1200.0;
    atmosphere.mieDensity[1].linearTerm = 0.0;
    atmosphere.mieDensity[1].constantTerm = 0.0;
    atmosphere.mieScattering = float3(3.996e-6, 3.996e-6, 3.996e-6);
    atmosphere.mieExtinction = float3(4.440e-6, 4.440e-6, 4.440e-6);
    atmosphere.miePhaseFunctionG = 0.8;

    // Absorption (Ozone)
    atmosphere.absorptionDensity[0].width = 15000.0;
    atmosphere.absorptionDensity[0].expTerm = 0.0;
    atmosphere.absorptionDensity[0].expScale = 0.0;
    atmosphere.absorptionDensity[0].linearTerm = 1.0 / 15000.0;
    atmosphere.absorptionDensity[0].constantTerm = -2.0 / 3.0;
    atmosphere.absorptionDensity[1].width = 0.0;
    atmosphere.absorptionDensity[1].expTerm = 0.0;
    atmosphere.absorptionDensity[1].expScale = 0.0;
    atmosphere.absorptionDensity[1].linearTerm = -1.0 / 15000.0;
    atmosphere.absorptionDensity[1].constantTerm = 8.0 / 3.0;
    atmosphere.absorptionExtinction = float3(0.650e-6, 1.881e-6, 0.085e-6);

    // Ground
    atmosphere.groundAlbedo = float3(0.1, 0.1, 0.1);
    atmosphere.muSMin = -0.2;

    return atmosphere;
}

// =============================================================================
// Main Fragment Shader
// =============================================================================

[shader("fragment")]
FragmentOutput FragmentMain(FragmentInput input)
{
    FragmentOutput output;

    // Get atmosphere parameters
    AtmosphereParametersGPU atmosphere = GetAtmosphereParams();

    // Camera position relative to planet center
    // Earth center is at uniforms.earthCenter in world space
    float3 camera = uniforms.cameraPosition - uniforms.earthCenter;

    // View ray direction (normalized)
    float3 viewRay = normalize(input.viewRay);

    // Sun direction
    float3 sunDir = normalize(uniforms.sunDirection);

    // Get sky radiance
    float3 transmittance;
    float3 radiance = GetSkyRadiance(
        atmosphere,
        TransmittanceTexture,
        ScatteringTexture,
        SingleMieScatteringTexture,
        LUTSampler,
        camera, viewRay, 0.0,
        sunDir, transmittance);

    // Add sun disk
    float cosSunAngle = dot(viewRay, sunDir);
    if (cosSunAngle > cos(atmosphere.sunAngularRadius))
    {
        // Sun disk rendering with smooth edge
        float sunEdge = smoothstep(
            cos(atmosphere.sunAngularRadius * 1.0),
            cos(atmosphere.sunAngularRadius * 0.9),
            cosSunAngle);
        float3 sunRadiance = GetSolarRadiance(atmosphere) * transmittance * sunEdge;
        radiance += sunRadiance;
    }

    // Tone mapping
    float3 color = ToneMap(radiance, uniforms.exposure, uniforms.whitePoint);

    // Output with full opacity
    output.color = float4(color, 1.0);

    return output;
}
