#include "Renderer/ForwardRenderer.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"

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
    Renderer::Shutdown();
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
    (void)options;

    _currentRenderTarget = renderTarget;
    const RenderTargetInfo& rtInfo = _currentRenderTarget->GetInfo();

    auto makeRenderingInfo = [&](const Attachment& colorAttachment,
                                 bool useDepthStencilAttachment,
                                 const Attachment& depthStencilAttachment) -> RenderingInfo
    {
        RenderingInfo renderingInfo{};
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.useDepthStencilAttachment = useDepthStencilAttachment;
        renderingInfo.isSwapchainRendering = false;
        renderingInfo.renderArea = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
        renderingInfo.enableAutomaticTransitions = false;

        RenderingAttachmentInfo colorAttachmentInfo{};
        colorAttachmentInfo.texture = _currentRenderTarget->GetColorTexture(0);
        colorAttachmentInfo.attachment = colorAttachment;
        renderingInfo.colorAttachments.push_back(colorAttachmentInfo);

        if (useDepthStencilAttachment)
        {
            renderingInfo.depthStencilAttachment.texture = _currentRenderTarget->GetDepthStencilTexture();
            renderingInfo.depthStencilAttachment.attachment = depthStencilAttachment;
        }

        return renderingInfo;
    };

    Attachment colorAttachment{};
    colorAttachment.format = rtInfo.colorTextureInfos[0].format;
    colorAttachment.clearValue = ClearValue(0.33f, 0.33f, 0.33f, 1.0f);
    colorAttachment.isDepthStencil = false;
    colorAttachment.loadAction = ELoadAction::Clear;
    colorAttachment.storeAction = EStoreAction::Store;

    Attachment depthAttachment{};
    bool useDepthStencilAttachment = false;
    if (rtInfo.useDepthStencilTexture)
    {
        depthAttachment.format = rtInfo.depthStencilInfo.format;
        depthAttachment.clearValue = ClearValue(1.0f, 0.0f);
        depthAttachment.isDepthStencil = true;
        depthAttachment.loadAction = ELoadAction::Clear;
        depthAttachment.storeAction = EStoreAction::Store;
        useDepthStencilAttachment = true;
    }

    SceneResource sceneResource = _resourceManager->BuildSceneResource(snapshot);
    RenderingInfo renderingInfo = makeRenderingInfo(colorAttachment, useDepthStencilAttachment, depthAttachment);
    PipelineRenderTargetLayout renderTargetLayout = renderingInfo.ToRenderTargetLayout();

    _graphBuilder.Setup(_curCommandBuffer);

    struct OpaquePassParameters
    {
    } opaqueParams;

    _graphBuilder.AddPass("Opaque", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &opaqueParams,
        [&](RenderGraphBuilder& builder, RGPass* pass, OpaquePassParameters*) -> void
        {
            RGTexture* colorTex = builder.RegisterExternalTexture(renderTarget->GetColorTexture(0));
            builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

            if (rtInfo.useDepthStencilTexture)
            {
                RGTexture* depthTex = builder.RegisterExternalTexture(renderTarget->GetDepthStencilTexture());
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

                uint32 vbOffset = 0;
                const RHIBuffer* vb = renderModel.meshResource->vertexBuffer;
                commandBuffer.BindVertexBuffers(&vb, &vbOffset, 1);
                commandBuffer.BindIndexBuffer(renderModel.meshResource->indexBuffer);
                commandBuffer.DrawIndexed(0, renderModel.meshResource->indexCount, 1, 0);
            }

            commandBuffer.PopDebugMark();
            commandBuffer.EndRendering();
        });

    _graphBuilder.Compile();
    _graphBuilder.Execute();
    _graphBuilder.Reset();
}

HS_NS_END
