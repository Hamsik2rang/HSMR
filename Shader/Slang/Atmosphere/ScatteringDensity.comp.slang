//
// ScatteringDensity.comp.slang
// Compute shader for scattering density computation (multi-scattering step)
//
// Computes the density of nth-order scattered light at each point.
// This is used to compute higher-order (n+1) scattering.
//
#include "AtmosphereCommon.hlsli"

// Atmosphere parameters uniform buffer
ConstantBuffer<AtmosphereParametersGPU> atmosphere;

// Input textures
Texture2D<float4> TransmittanceTexture;
Texture3D<float4> SingleRayleighScatteringTexture;
Texture3D<float4> SingleMieScatteringTexture;
Texture3D<float4> MultipleScatteringTexture;
Texture2D<float4> IrradianceTexture;
SamplerState LinearSampler;

// Output scattering density texture
RWTexture3D<float4> ScatteringDensityTexture;

// Number of integration samples
static const int SPHERICAL_SAMPLES = 16;

// Get transmittance
float3 GetTransmittance(float r, float mu)
{
    float2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
    return TransmittanceTexture.SampleLevel(LinearSampler, uv, 0).rgb;
}

// Get scattering from textures
// Note: MultipleScatteringTexture should be zeroed for order 2, so no condition needed
float3 GetScattering(float r, float mu, float mu_s, float nu, bool rayIntersectsGround)
{
    float3 uvw = GetScatteringTextureUvwFromRMuMuSNu(atmosphere, r, mu, mu_s, nu, rayIntersectsGround);

    float3 rayleigh = SingleRayleighScatteringTexture.SampleLevel(LinearSampler, uvw, 0).rgb;
    float3 mie = SingleMieScatteringTexture.SampleLevel(LinearSampler, uvw, 0).rgb;
    float3 multipleScattering = MultipleScatteringTexture.SampleLevel(LinearSampler, uvw, 0).rgb;

    return (rayleigh + multipleScattering) * RayleighPhaseFunction(nu) + mie * MiePhaseFunction(atmosphere.miePhaseFunctionG, nu);
}

// Get irradiance
float3 GetIrradiance(float r, float mu_s)
{
    float2 uv = GetIrradianceTextureUvFromRMuS(atmosphere, r, mu_s);
    return IrradianceTexture.SampleLevel(LinearSampler, uv, 0).rgb;
}

// Compute scattering density
float3 ComputeScatteringDensity(float r, float mu, float mu_s, float nu)
{
    float3 rayleigh_mie = float3(0.0, 0.0, 0.0);

    // Zenith angle
    float3 zenith_direction = float3(0.0, 0.0, 1.0);
    float3 omega = float3(sqrt(1.0 - mu * mu), 0.0, mu);
    float sun_dir_x = sqrt(1.0 - mu_s * mu_s);
    float sun_dir_y = 0.0;
    float sun_dir_z = mu_s;
    float3 sun_direction = float3(sun_dir_x, sun_dir_y, sun_dir_z);

    // Integrate over all incoming directions
    for (int l = 0; l < SPHERICAL_SAMPLES; l++)
    {
        float theta = (float(l) + 0.5) * PI / float(SPHERICAL_SAMPLES);
        float cos_theta = cos(theta);
        float sin_theta = sin(theta);
        bool groundReflection = cos_theta < 0.0;

        // Distance to nearest boundary
        float dist = groundReflection
            ? DistanceToBottomAtmosphereBoundary(atmosphere, r, cos_theta)
            : DistanceToTopAtmosphereBoundary(atmosphere, r, cos_theta);

        // Skip if negligible distance
        if (dist <= 0.0) continue;

        for (int m = 0; m < 2 * SPHERICAL_SAMPLES; m++)
        {
            float phi = (float(m) + 0.5) * PI / float(SPHERICAL_SAMPLES);

            float3 omega_i = float3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
            float domega_i = (PI / float(SPHERICAL_SAMPLES)) *
                            (PI / float(SPHERICAL_SAMPLES)) * sin_theta;

            // Scattering angles
            float nu1 = dot(omega, omega_i);
            float mu_i = omega_i.z;
            float mu_s_i = dot(sun_direction, omega_i);

            if (groundReflection)
            {
                // Ground reflected light
                float r_ground = atmosphere.bottomRadius;
                float3 groundNormal = float3(omega_i.x, omega_i.y, 0.0);
                groundNormal = normalize(groundNormal);

                float3 groundIrradiance = GetIrradiance(r_ground, mu_s_i);
                float3 groundReflectance = atmosphere.groundAlbedo / PI;
                float3 groundRadiance = groundReflectance * groundIrradiance;

                // Transmit back to r
                float3 transmittance = GetTransmittance(r_ground, -mu_i);
                rayleigh_mie += transmittance * groundRadiance *
                               (atmosphere.rayleighScattering * RayleighPhaseFunction(nu1) +
                                atmosphere.mieScattering * MiePhaseFunction(atmosphere.miePhaseFunctionG, nu1)) *
                               domega_i;
            }
            else
            {
                // Scattered light from atmosphere
                float3 scattering_i = GetScattering(r, mu_i, mu_s_i, nu1, false);
                rayleigh_mie += scattering_i *
                               (atmosphere.rayleighScattering * RayleighPhaseFunction(nu) +
                                atmosphere.mieScattering * MiePhaseFunction(atmosphere.miePhaseFunctionG, nu)) *
                               domega_i;
            }
        }
    }

    return rayleigh_mie;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void ComputeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Check bounds
    if (dispatchThreadID.x >= SCATTERING_TEXTURE_WIDTH ||
        dispatchThreadID.y >= SCATTERING_TEXTURE_HEIGHT ||
        dispatchThreadID.z >= SCATTERING_TEXTURE_DEPTH)
    {
        return;
    }

    // Convert to UVW
    float3 uvw = (float3(dispatchThreadID) + 0.5) /
                 float3(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH);

    // Get (r, mu, mu_s, nu)
    float r, mu, mu_s, nu;
    bool rayIntersectsGround;
    GetRMuMuSNuFromScatteringTextureUvw(atmosphere, uvw, r, mu, mu_s, nu, rayIntersectsGround);

    // Compute scattering density
    float3 scatteringDensity = ComputeScatteringDensity(r, mu, mu_s, nu);

    ScatteringDensityTexture[dispatchThreadID] = float4(scatteringDensity, 1.0);
}
