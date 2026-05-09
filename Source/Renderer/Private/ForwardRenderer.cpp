#include "Renderer/ForwardRenderer.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"

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

    auto makeRenderingInfo = [&](const Attachment& colorAttachment,
                                 bool useDepthStencilAttachment,
                                 const Attachment& depthStencilAttachment) -> RenderingInfo
    {
        RenderingInfo renderingInfo{};
        renderingInfo.colorAttachmentCount       = 1;
        renderingInfo.useDepthStencilAttachment  = useDepthStencilAttachment;
        renderingInfo.isSwapchainRendering       = false;
        renderingInfo.renderArea                 = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
        renderingInfo.enableAutomaticTransitions = false;

        RenderingAttachmentInfo colorAttachmentInfo{};
        colorAttachmentInfo.texture    = _currentRenderTarget->GetColorTexture(0);
        colorAttachmentInfo.attachment = colorAttachment;
        renderingInfo.colorAttachments.push_back(colorAttachmentInfo);

        if (useDepthStencilAttachment)
        {
            renderingInfo.depthStencilAttachment.texture    = _currentRenderTarget->GetDepthStencilTexture();
            renderingInfo.depthStencilAttachment.attachment = depthStencilAttachment;
        }

        return renderingInfo;
    };

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
        RGTexture* colorTex = builder.RegisterExternalTexture(renderTarget->GetColorTexture(0), colorFinalState);
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
