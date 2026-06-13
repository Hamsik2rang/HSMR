#include "Renderer/ForwardRenderer.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"

#include "Core/HAL/Timer.h"

#include "Renderer/ShaderLibrary.h"

#include "Resource/Image.h"

HS_NS_BEGIN

ForwardRenderer::ForwardRenderer(RHIContext* rhiContext)
    : Renderer(rhiContext)
{
}

ForwardRenderer::~ForwardRenderer()
{
}

void ForwardRenderer::Shutdown()
{
    _volumetricCloudPass.reset();
    _atmospherePass.reset();
    _skyboxPass.reset();
    Renderer::Shutdown();
}

bool ForwardRenderer::SetSkybox(const std::array<Image*, 6>& faces)
{
    if (!_skyboxPass)
    {
        _skyboxPass = MakeScoped<ForwardSkyboxPass>();
        if (!_skyboxPass->Initialize(_shaderLibrary, _rhiContext))
        {
            _skyboxPass.reset();
            return false;
        }
    }
    return _skyboxPass->ConfigureSixSided(faces);
}

void ForwardRenderer::ClearSkybox()
{
    _skyboxPass.reset();
}

void ForwardRenderer::Render(Scene* scene, RenderTarget* renderTarget)
{
    RenderSceneSnapshot sceneSnapshot = _resourceManager->BuildRenderSceneSnapshot(scene, _shaderLibrary);
    Render(sceneSnapshot, renderTarget, RenderOptions{});
}

void ForwardRenderer::Render(Scene* scene, RenderTarget* renderTarget, const RenderOptions& options)
{
    RenderSceneSnapshot sceneSnapshot = _resourceManager->BuildRenderSceneSnapshot(scene, _shaderLibrary);
    Render(sceneSnapshot, renderTarget, options);
}

void ForwardRenderer::Render(const RenderSceneSnapshot& snapshot, RenderTarget* renderTarget)
{
    Render(snapshot, renderTarget, RenderOptions{});
}

void ForwardRenderer::Render(const RenderSceneSnapshot& snapshot, RenderTarget* renderTarget, const RenderOptions& options)
{
    _currentRenderTarget           = renderTarget;
    const RenderTargetInfo& rtInfo = _currentRenderTarget->GetInfo();

    Attachment colorAttachment{};
    colorAttachment.format         = rtInfo.colorTextureInfo[0].format;
    colorAttachment.clearValue     = ClearValue(0.33f, 0.33f, 0.33f, 1.0f);
    colorAttachment.isDepthStencil = false;
    colorAttachment.loadAction     = ELoadAction::Clear;
    colorAttachment.storeAction    = EStoreAction::Store;

    Attachment depthAttachment{};
    bool useDepthStencilAttachment = false;
    if (rtInfo.useDepthStencilTexture)
    {
        depthAttachment.format         = rtInfo.depthStencilInfo.format;
        depthAttachment.clearValue     = ClearValue(1.0f, 0.0f);
        depthAttachment.isDepthStencil = true;
        depthAttachment.loadAction     = ELoadAction::Clear;
        depthAttachment.storeAction    = EStoreAction::Store;
        useDepthStencilAttachment      = true;
    }

    SceneResource sceneResource                   = _resourceManager->BuildSceneResource(snapshot);
    RenderingInfo renderingInfo                   = makeRenderingInfo(colorAttachment, useDepthStencilAttachment, depthAttachment);
    PipelineRenderTargetLayout renderTargetLayout = renderingInfo.ToRenderTargetLayout();

    _graphBuilder.Setup(_curCommandBuffer);

    struct OpaquePassParameters
    {
        // Empty
    } opaqueParams;

    _graphBuilder.AddPass("Opaque", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &opaqueParams,
        [&](RenderGraphBuilder& builder, RGPass* pass, OpaquePassParameters*) -> void
    {
        // Swapchain color는 마지막에 Present 레이아웃으로 가야 하고(SAMPLED_BIT 없음),
        // panel offscreen RT는 ImGui sampling을 위해 ReadOnly(SHADER_READ_ONLY)로 전환되어야 함.
        const ERGTextureAccess colorFinalState = rtInfo.isSwapchainTarget
                                                     ? ERGTextureAccess::Present
                                                     : ERGTextureAccess::ReadOnly;
        RGTexture* colorTex                    = builder.RegisterExternalTexture(renderTarget->GetColorTexture(0), colorFinalState);
        builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

        if (rtInfo.useDepthStencilTexture)
        {
            // Depth는 sampled되지 않고 다음 프레임에도 attachment로 사용되므로 attachment 상태로 유지.
            RGTexture* depthTex = builder.RegisterExternalTexture(
                renderTarget->GetDepthStencilTexture(),
                ERGTextureAccess::DepthAttachmentWrite);
            builder.Write(pass, depthTex, ERGTextureAccess::DepthAttachmentWrite);
        }
    },
        [&](RHICommandBuffer& commandBuffer) -> void
    {
        RenderResourceManager* resMgr = GetResourceManager();
        float debugColor[4]{0.2f, 0.5f, 0.8f, 1.0f};

        commandBuffer.BeginRendering(renderingInfo);
        commandBuffer.PushDebugMark("Opaque Pass", debugColor);
        commandBuffer.SetViewport(Viewport{
            0.0f, 0.0f,
            static_cast<float>(_currentRenderTarget->GetWidth()),
            static_cast<float>(_currentRenderTarget->GetHeight()),
            0.0f, 1.0f});
        commandBuffer.SetScissor(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());

        for (const auto& renderModel : sceneResource.renderModels)
        {
            Material* mat = renderModel.material;
            if (!mat)
            {
                continue;
            }

            RHIGraphicsPipeline* pipeline = resMgr->GetOrCreatePipeline(mat, renderTargetLayout);
            if (!pipeline)
            {
                continue;
            }

            commandBuffer.BindPipeline(pipeline);
            if (!renderModel.drawResource || !renderModel.drawResource->resourceSet)
            {
                continue;
            }

            commandBuffer.BindResourceSet(renderModel.drawResource->resourceSet);

            uint32 vbOffset     = 0;
            const RHIBuffer* vb = renderModel.meshResource->vertexBuffer;
            commandBuffer.BindVertexBuffers(&vb, &vbOffset, 1);
            commandBuffer.BindIndexBuffer(renderModel.meshResource->indexBuffer);
            commandBuffer.DrawIndexed(0, renderModel.meshResource->indexCount, 1, 0);
        }

        commandBuffer.PopDebugMark();
        commandBuffer.EndRendering();
    });

    const bool hasCamera = !sceneResource.cameraResources.empty();
    const bool needsAtmosphere = hasCamera && (options.enableAtmosphere || options.enableVolumetricClouds);
    if (needsAtmosphere)
    {
        if (!_atmospherePass)
        {
            _atmospherePass = MakeScoped<AtmospherePass>();
            if (!_atmospherePass->Initialize(_shaderLibrary, _rhiContext))
            {
                _atmospherePass.reset();
            }
        }

        if (_atmospherePass && _atmospherePass->IsInitialized())
        {
            _atmospherePass->UpdateSettings(options.atmosphere);

            struct AtmospherePrecomputePassParameters
            {
            } atmospherePrecomputeParams;

            _graphBuilder.AddPass("AtmospherePrecompute", ERGPassFlag::Compute | ERGPassFlag::NeverCull,
                &atmospherePrecomputeParams,
                [&](RenderGraphBuilder&, RGPass*, AtmospherePrecomputePassParameters*) -> void
                {
                },
                [&](RHICommandBuffer& commandBuffer) -> void
                {
                    float debugColor[4]{0.2f, 0.45f, 1.0f, 1.0f};
                    commandBuffer.PushDebugMark("Atmosphere Precompute", debugColor);
                    _atmospherePass->PrecomputeIfNeeded(commandBuffer);
                    commandBuffer.PopDebugMark();
                });
        }
    }

    if (options.enableAtmosphere && _atmospherePass && _atmospherePass->IsInitialized() && hasCamera)
    {
        const ERGTextureAccess colorFinalState = rtInfo.isSwapchainTarget
                                                     ? ERGTextureAccess::Present
                                                     : ERGTextureAccess::ReadOnly;

        RenderingInfo skyInfo{};
        skyInfo.colorAttachmentCount = 1;
        skyInfo.useDepthStencilAttachment = useDepthStencilAttachment;
        skyInfo.isSwapchainRendering = false;
        skyInfo.renderArea = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
        skyInfo.enableAutomaticTransitions = false;

        Attachment skyColor{};
        skyColor.format = rtInfo.colorTextureInfo[0].format;
        skyColor.clearValue = ClearValue(0.0f, 0.0f, 0.0f, 1.0f);
        skyColor.isDepthStencil = false;
        skyColor.loadAction = ELoadAction::Load;
        skyColor.storeAction = EStoreAction::Store;

        RenderingAttachmentInfo skyColorAttachment{};
        skyColorAttachment.texture = _currentRenderTarget->GetColorTexture(0);
        skyColorAttachment.attachment = skyColor;
        skyInfo.colorAttachments.push_back(skyColorAttachment);

        if (useDepthStencilAttachment)
        {
            Attachment skyDepth{};
            skyDepth.format = rtInfo.depthStencilInfo.format;
            skyDepth.clearValue = ClearValue(1.0f, 0.0f);
            skyDepth.isDepthStencil = true;
            skyDepth.loadAction = ELoadAction::Load;
            skyDepth.storeAction = EStoreAction::Store;
            skyInfo.depthStencilAttachment.texture = _currentRenderTarget->GetDepthStencilTexture();
            skyInfo.depthStencilAttachment.attachment = skyDepth;
        }

        PipelineRenderTargetLayout skyRTLayout = skyInfo.ToRenderTargetLayout();

        struct AtmosphereSkyPassParameters
        {
        } atmosphereSkyParams;

        _graphBuilder.AddPass("AtmosphereSky", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &atmosphereSkyParams,
            [&](RenderGraphBuilder& builder, RGPass* pass, AtmosphereSkyPassParameters*) -> void
            {
                RGTexture* colorTex = builder.RegisterExternalTexture(_currentRenderTarget->GetColorTexture(0), colorFinalState);
                builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

                if (useDepthStencilAttachment)
                {
                    RGTexture* depthTex = builder.RegisterExternalTexture(
                        _currentRenderTarget->GetDepthStencilTexture(),
                        ERGTextureAccess::DepthAttachmentWrite);
                    builder.Read(pass, depthTex, ERGTextureAccess::DepthAttachmentRead);
                }
            },
            [&, skyRTLayout, skyInfo](RHICommandBuffer& commandBuffer) -> void
            {
                RHIGraphicsPipeline* pipeline = _atmospherePass->GetOrCreateSkyPipeline(
                    skyRTLayout,
                    sceneResource.cameraResources[0]->perViewBuffer);
                if (!pipeline || !_atmospherePass->GetSkyResourceSet())
                {
                    return;
                }

                float debugColor[4]{0.2f, 0.45f, 1.0f, 1.0f};
                commandBuffer.BeginRendering(skyInfo);
                commandBuffer.PushDebugMark("Atmosphere Sky Pass", debugColor);
                commandBuffer.SetViewport(Viewport{
                    0.0f, 0.0f,
                    static_cast<float>(_currentRenderTarget->GetWidth()),
                    static_cast<float>(_currentRenderTarget->GetHeight()),
                    0.0f, 1.0f});
                commandBuffer.SetScissor(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
                commandBuffer.BindPipeline(pipeline);
                commandBuffer.BindResourceSet(_atmospherePass->GetSkyResourceSet());
                commandBuffer.DrawArrays(0, 3, 1);
                commandBuffer.PopDebugMark();
                commandBuffer.EndRendering();
            });
    }

    if (options.enableVolumetricClouds && !sceneResource.cameraResources.empty())
    {
        if (!_volumetricCloudPass)
        {
            _volumetricCloudPass = MakeScoped<VolumetricCloudPass>();
            if (!_volumetricCloudPass->Initialize(_shaderLibrary, _rhiContext))
            {
                _volumetricCloudPass.reset();
            }
        }

        if (_volumetricCloudPass && _volumetricCloudPass->IsInitialized())
        {
            const ERGTextureAccess colorFinalState = rtInfo.isSwapchainTarget
                                                         ? ERGTextureAccess::Present
                                                         : ERGTextureAccess::ReadOnly;

            RenderingInfo cloudInfo{};
            cloudInfo.colorAttachmentCount       = 1;
            cloudInfo.useDepthStencilAttachment  = useDepthStencilAttachment;
            cloudInfo.isSwapchainRendering       = false;
            cloudInfo.renderArea                 = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
            cloudInfo.enableAutomaticTransitions = false;

            Attachment cloudColor{};
            cloudColor.format         = rtInfo.colorTextureInfo[0].format;
            cloudColor.clearValue     = ClearValue(0.0f, 0.0f, 0.0f, 1.0f);
            cloudColor.isDepthStencil = false;
            cloudColor.loadAction     = ELoadAction::Load;
            cloudColor.storeAction    = EStoreAction::Store;

            RenderingAttachmentInfo cloudColorAttachment{};
            cloudColorAttachment.texture    = _currentRenderTarget->GetColorTexture(0);
            cloudColorAttachment.attachment = cloudColor;
            cloudInfo.colorAttachments.push_back(cloudColorAttachment);

            if (useDepthStencilAttachment)
            {
                Attachment cloudDepth{};
                cloudDepth.format         = rtInfo.depthStencilInfo.format;
                cloudDepth.clearValue     = ClearValue(1.0f, 0.0f);
                cloudDepth.isDepthStencil = true;
                cloudDepth.loadAction     = ELoadAction::Load;
                cloudDepth.storeAction    = EStoreAction::Store;
                cloudInfo.depthStencilAttachment.texture    = _currentRenderTarget->GetDepthStencilTexture();
                cloudInfo.depthStencilAttachment.attachment = cloudDepth;
            }

            PipelineRenderTargetLayout cloudRTLayout = cloudInfo.ToRenderTargetLayout();

            struct VolumetricCloudPassParameters
            {
            } cloudParams;

            _graphBuilder.AddPass("VolumetricCloud", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &cloudParams,
                [&](RenderGraphBuilder& builder, RGPass* pass, VolumetricCloudPassParameters*) -> void
                {
                    RGTexture* colorTex = builder.RegisterExternalTexture(_currentRenderTarget->GetColorTexture(0), colorFinalState);
                    builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

                    if (useDepthStencilAttachment)
                    {
                        RGTexture* depthTex = builder.RegisterExternalTexture(
                            _currentRenderTarget->GetDepthStencilTexture(),
                            ERGTextureAccess::DepthAttachmentWrite);
                        builder.Read(pass, depthTex, ERGTextureAccess::DepthAttachmentRead);
                    }
                },
                [&, cloudRTLayout, cloudInfo](RHICommandBuffer& commandBuffer) -> void
                {
                    float timeSeconds = static_cast<float>(Timer::GetElapsedSeconds());
                    _volumetricCloudPass->UpdateSettings(options.volumetricClouds, timeSeconds);
                    if (_atmospherePass && _atmospherePass->IsInitialized())
                    {
                        _volumetricCloudPass->SetAtmosphereResources(_atmospherePass->GetLutResources());
                    }

                    RHIGraphicsPipeline* pipeline = _volumetricCloudPass->GetOrCreatePipeline(
                        cloudRTLayout,
                        sceneResource.cameraResources[0]->perViewBuffer);
                    if (!pipeline || !_volumetricCloudPass->GetResourceSet())
                    {
                        return;
                    }

                    float debugColor[4]{0.45f, 0.62f, 1.0f, 1.0f};
                    commandBuffer.BeginRendering(cloudInfo);
                    commandBuffer.PushDebugMark("Volumetric Cloud Pass", debugColor);
                    commandBuffer.SetViewport(Viewport{
                        0.0f, 0.0f,
                        static_cast<float>(_currentRenderTarget->GetWidth()),
                        static_cast<float>(_currentRenderTarget->GetHeight()),
                        0.0f, 1.0f});
                    commandBuffer.SetScissor(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
                    commandBuffer.BindPipeline(pipeline);
                    commandBuffer.BindResourceSet(_volumetricCloudPass->GetResourceSet());
                    commandBuffer.DrawArrays(0, 3, 1);
                    commandBuffer.PopDebugMark();
                    commandBuffer.EndRendering();
                });
        }
    }

    // Skybox pass: rendered after opaque so it only fills unwritten (depth==1.0) background pixels.
    // if (_skyboxPass && _skyboxPass->HasSkybox() && !sceneResource.cameraResources.empty())
    //{
    //    RenderingInfo skyboxInfo{};
    //    skyboxInfo.colorAttachmentCount       = 1;
    //    skyboxInfo.useDepthStencilAttachment  = useDepthStencilAttachment;
    //    skyboxInfo.isSwapchainRendering       = false;
    //    skyboxInfo.renderArea                 = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
    //    skyboxInfo.enableAutomaticTransitions = false;

    //    Attachment skyboxColor{};
    //    skyboxColor.format         = rtInfo.colorTextureInfo[0].format;
    //    skyboxColor.clearValue     = ClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    //    skyboxColor.isDepthStencil = false;
    //    skyboxColor.loadAction     = ELoadAction::Load;
    //    skyboxColor.storeAction    = EStoreAction::Store;

    //    RenderingAttachmentInfo skyboxColorAttachment{};
    //    skyboxColorAttachment.texture    = _currentRenderTarget->GetColorTexture(0);
    //    skyboxColorAttachment.attachment = skyboxColor;
    //    skyboxInfo.colorAttachments.push_back(skyboxColorAttachment);

    //    if (useDepthStencilAttachment)
    //    {
    //        Attachment skyboxDepth{};
    //        skyboxDepth.format         = rtInfo.depthStencilInfo.format;
    //        skyboxDepth.clearValue     = ClearValue(1.0f, 0.0f);
    //        skyboxDepth.isDepthStencil = true;
    //        skyboxDepth.loadAction     = ELoadAction::Load;
    //        skyboxDepth.storeAction    = EStoreAction::Store;
    //        skyboxInfo.depthStencilAttachment.texture    = _currentRenderTarget->GetDepthStencilTexture();
    //        skyboxInfo.depthStencilAttachment.attachment = skyboxDepth;
    //    }

    //    PipelineRenderTargetLayout skyboxRTLayout = skyboxInfo.ToRenderTargetLayout();

    //    struct SkyboxPassParameters {} skyboxParams;

    //    _graphBuilder.AddPass("Skybox", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &skyboxParams,
    //        [&](RenderGraphBuilder& builder, RGPass* pass, SkyboxPassParameters*) -> void
    //        {
    //            RGTexture* colorTex = builder.RegisterExternalTexture(_currentRenderTarget->GetColorTexture(0));
    //            builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

    //            if (useDepthStencilAttachment)
    //            {
    //                RGTexture* depthTex = builder.RegisterExternalTexture(_currentRenderTarget->GetDepthStencilTexture());
    //                builder.Read(pass, depthTex, ERGTextureAccess::DepthAttachmentRead);
    //            }
    //        },
    //        [&](RHICommandBuffer& commandBuffer) -> void
    //        {
    //            RHIGraphicsPipeline* pipeline = _skyboxPass->GetOrCreatePipeline(
    //                skyboxRTLayout,
    //                sceneResource.cameraResources[0]->perViewBuffer);
    //            if (!pipeline || !_skyboxPass->GetResourceSet()) return;

    //            float debugColor[4]{0.6f, 0.8f, 1.0f, 1.0f};
    //            commandBuffer.BeginRendering(skyboxInfo);
    //            commandBuffer.PushDebugMark("Skybox Pass", debugColor);
    //            commandBuffer.SetViewport(Viewport{0.0f, 0.0f,
    //                static_cast<float>(_currentRenderTarget->GetWidth()),
    //                static_cast<float>(_currentRenderTarget->GetHeight()),
    //                0.0f, 1.0f});
    //            commandBuffer.SetScissor(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
    //            commandBuffer.BindPipeline(pipeline);
    //            commandBuffer.BindResourceSet(_skyboxPass->GetResourceSet());
    //            commandBuffer.DrawArrays(0, 3, 1);
    //            commandBuffer.PopDebugMark();
    //            commandBuffer.EndRendering();
    //        });
    //}
    _graphBuilder.Compile();
    _graphBuilder.Execute();
    _graphBuilder.Reset();
}

HS_NS_END
