#ifndef __HS_FORWARD_RENDERER_H__
#define __HS_FORWARD_RENDERER_H__

#include "Precompile.h"

#include "Renderer/Renderer.h"
#include "Renderer/RenderPass/AtmospherePass.h"
#include "Renderer/RenderPass/ForwardSkyboxPass.h"
#include "Renderer/RenderPass/VolumetricCloudPass.h"

#include <array>

HS_NS_BEGIN

class Image;

class HS_RENDERER_API ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(RHIContext* rhiContext);
    ~ForwardRenderer() override;

    void Render(Scene* scene, RenderTarget* renderTarget) override;
    void Render(Scene* scene, RenderTarget* renderTarget, const RenderOptions& options) override;
    void Render(const RenderSceneSnapshot& snapshot, RenderTarget* renderTarget) override;
    void Render(const RenderSceneSnapshot& snapshot, RenderTarget* renderTarget, const RenderOptions& options) override;
    void Shutdown() override;

    // Configure the skybox from 6 separate 2D images.
    // Face order: +X, -X, +Y, -Y, +Z, -Z. Must be called after Initialize().
    bool SetSkybox(const std::array<Image*, 6>& faces);

    void ClearSkybox();

private:
    Scoped<AtmospherePass> _atmospherePass;
    Scoped<ForwardSkyboxPass> _skyboxPass;
    Scoped<VolumetricCloudPass> _volumetricCloudPass;
};

HS_NS_END

#endif /* __HS_FORWARD_RENDERER_H__ */
