#include "Renderer/ForwardOverlayRenderer.h"

#include "RHI/CommandHandle.h"
#include "Renderer/RenderTarget.h"
#include "Renderer/RenderResourceManager.h"
#include "Renderer/ShaderLibrary.h"

HS_NS_BEGIN

namespace
{
RenderSceneSnapshot buildSingleViewSnapshot(const RenderSceneSnapshot& snapshot, const RenderViewSnapshot& viewSnapshot)
{
    RenderSceneSnapshot singleViewSnapshot = snapshot;
    singleViewSnapshot.views.clear();
    singleViewSnapshot.views.push_back(viewSnapshot);
    return singleViewSnapshot;
}
}

ForwardOverlayRenderer::ForwardOverlayRenderer(RHIContext* rhiContext)
    : _rhiContext(rhiContext)
{
}

ForwardOverlayRenderer::~ForwardOverlayRenderer()
{
    Shutdown();
}

bool ForwardOverlayRenderer::Initialize(ShaderLibrary* shaderLibrary)
{
    _shaderLibrary = shaderLibrary;
    _graphBuilder.Initialize(_rhiContext);
    _isInitialized = true;
    return true;
}

void ForwardOverlayRenderer::Shutdown()
{
    _debugPass.reset();
    _gridPass.reset();
    _shaderLibrary = nullptr;
    _isInitialized = false;
}

void ForwardOverlayRenderer::Render(
    RHICommandBuffer& commandBuffer,
    RenderResourceManager& resourceManager,
    const RenderSceneSnapshot& snapshot,
    const RenderViewSnapshot& viewSnapshot,
    RenderTarget* renderTarget,
    const RenderOptions& options)
{
    if (!_isInitialized || !renderTarget || (!options.enableGrid && !options.enableDebug))
    {
        return;
    }

    RenderSceneSnapshot singleViewSnapshot = buildSingleViewSnapshot(snapshot, viewSnapshot);
    SceneResource sceneResource = resourceManager.BuildSceneResource(singleViewSnapshot);
    if (sceneResource.cameraResources.empty())
    {
        return;
    }

    const RenderTargetInfo& rtInfo = renderTarget->GetInfo();

    auto makeRenderingInfo = [&](const Attachment& colorAttachment,
                                 bool useDepthStencilAttachment,
                                 const Attachment& depthStencilAttachment) -> RenderingInfo
    {
        RenderingInfo renderingInfo{};
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.useDepthStencilAttachment = useDepthStencilAttachment;
        renderingInfo.isSwapchainRendering = false;
        renderingInfo.renderArea = Area(0, 0, renderTarget->GetWidth(), renderTarget->GetHeight());
        renderingInfo.enableAutomaticTransitions = false;

        RenderingAttachmentInfo colorAttachmentInfo{};
        colorAttachmentInfo.texture = renderTarget->GetColorTexture(0);
        colorAttachmentInfo.attachment = colorAttachment;
        renderingInfo.colorAttachments.push_back(colorAttachmentInfo);

        if (useDepthStencilAttachment)
        {
            renderingInfo.depthStencilAttachment.texture = renderTarget->GetDepthStencilTexture();
            renderingInfo.depthStencilAttachment.attachment = depthStencilAttachment;
        }

        return renderingInfo;
    };

    Attachment colorAttachment{};
    colorAttachment.format = rtInfo.colorTextureInfos[0].format;
    colorAttachment.clearValue = ClearValue(0.0f, 0.0f, 0.0f, 0.0f);
    colorAttachment.isDepthStencil = false;
    colorAttachment.loadAction = ELoadAction::Load;
    colorAttachment.storeAction = EStoreAction::Store;

    Attachment depthAttachment{};
    bool useDepthStencilAttachment = false;
    if (rtInfo.useDepthStencilTexture)
    {
        depthAttachment.format = rtInfo.depthStencilInfo.format;
        depthAttachment.clearValue = ClearValue(1.0f, 0.0f);
        depthAttachment.isDepthStencil = true;
        depthAttachment.loadAction = ELoadAction::Load;
        depthAttachment.storeAction = EStoreAction::Store;
        useDepthStencilAttachment = true;
    }

    RenderingInfo renderingInfo = makeRenderingInfo(colorAttachment, useDepthStencilAttachment, depthAttachment);
    PipelineRenderTargetLayout renderTargetLayout = renderingInfo.ToRenderTargetLayout();

    if (options.enableGrid && !_gridPass)
    {
        _gridPass = MakeScoped<ForwardGridPass>();
        _gridPass->Initialize(_shaderLibrary, _rhiContext);
    }

    bool hasDebugDrawData = false;
    if (options.enableDebug)
    {
        if (!_debugPass)
        {
            _debugPass = MakeScoped<ForwardDebugPass>();
            _debugPass->Initialize(_shaderLibrary, _rhiContext);
        }

        if (_debugPass && _debugPass->IsInitialized())
        {
            hasDebugDrawData = _debugPass->Prepare(snapshot) && _debugPass->HasDrawData();
        }
    }

    _graphBuilder.Setup(&commandBuffer);

    if (options.enableGrid && _gridPass && _gridPass->IsInitialized())
    {
        struct GridPassParameters
        {
        } gridParams;

        _graphBuilder.AddPass("EditorGrid", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &gridParams,
            [&](RenderGraphBuilder& builder, RGPass* pass, GridPassParameters*) -> void
            {
                RGTexture* colorTex = builder.RegisterExternalTexture(renderTarget->GetColorTexture(0));
                builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

                if (rtInfo.useDepthStencilTexture)
                {
                    RGTexture* depthTex = builder.RegisterExternalTexture(renderTarget->GetDepthStencilTexture());
                    builder.Read(pass, depthTex, ERGTextureAccess::DepthAttachmentRead);
                }
            },
            [&](RHICommandBuffer& commandBufferRef) -> void
            {
                RHIGraphicsPipeline* pipeline = _gridPass->GetOrCreatePipeline(
                    renderTargetLayout,
                    sceneResource.cameraResources[0]->perViewBuffer);
                if (!pipeline)
                {
                    return;
                }

                float debugColor[4]{0.4f, 0.8f, 0.4f, 1.0f};
                commandBufferRef.BeginRendering(renderingInfo);
                commandBufferRef.PushDebugMark("Grid Pass", debugColor);
                commandBufferRef.SetViewport(Viewport{
                    0.0f, 0.0f,
                    static_cast<float>(renderTarget->GetWidth()),
                    static_cast<float>(renderTarget->GetHeight()),
                    0.0f, 1.0f});
                commandBufferRef.SetScissor(0, 0, renderTarget->GetWidth(), renderTarget->GetHeight());
                commandBufferRef.BindPipeline(pipeline);
                commandBufferRef.BindResourceSet(_gridPass->GetResourceSet());
                commandBufferRef.DrawArrays(0, 6, 1);
                commandBufferRef.PopDebugMark();
                commandBufferRef.EndRendering();
            });
    }

    if (options.enableDebug && hasDebugDrawData && _debugPass && _debugPass->IsInitialized())
    {
        struct DebugPassParameters
        {
        } debugParams;

        _graphBuilder.AddPass("EditorDebug", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &debugParams,
            [&](RenderGraphBuilder& builder, RGPass* pass, DebugPassParameters*) -> void
            {
                RGTexture* colorTex = builder.RegisterExternalTexture(renderTarget->GetColorTexture(0));
                builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

                if (rtInfo.useDepthStencilTexture)
                {
                    RGTexture* depthTex = builder.RegisterExternalTexture(renderTarget->GetDepthStencilTexture());
                    builder.Read(pass, depthTex, ERGTextureAccess::DepthAttachmentRead);
                }
            },
            [&](RHICommandBuffer& commandBufferRef) -> void
            {
                RHIGraphicsPipeline* pipeline = _debugPass->GetOrCreatePipeline(
                    renderTargetLayout,
                    sceneResource.cameraResources[0]->perViewBuffer);
                if (!pipeline || !_debugPass->GetResourceSet() || !_debugPass->GetVertexBuffer())
                {
                    return;
                }

                float debugColor[4]{1.0f, 0.8f, 0.2f, 1.0f};
                commandBufferRef.BeginRendering(renderingInfo);
                commandBufferRef.PushDebugMark("Debug Pass", debugColor);
                commandBufferRef.SetViewport(Viewport{
                    0.0f, 0.0f,
                    static_cast<float>(renderTarget->GetWidth()),
                    static_cast<float>(renderTarget->GetHeight()),
                    0.0f, 1.0f});
                commandBufferRef.SetScissor(0, 0, renderTarget->GetWidth(), renderTarget->GetHeight());
                commandBufferRef.BindPipeline(pipeline);
                commandBufferRef.BindResourceSet(_debugPass->GetResourceSet());

                uint32 vbOffset = 0;
                const RHIBuffer* vertexBuffer = _debugPass->GetVertexBuffer();
                commandBufferRef.BindVertexBuffers(&vertexBuffer, &vbOffset, 1);
                commandBufferRef.DrawArrays(0, _debugPass->GetVertexCount(), 1);
                commandBufferRef.PopDebugMark();
                commandBufferRef.EndRendering();
            });
    }

    _graphBuilder.Compile();
    _graphBuilder.Execute();
    _graphBuilder.Reset();
}

HS_NS_END
