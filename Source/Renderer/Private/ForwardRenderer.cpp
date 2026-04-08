#include "Renderer/ForwardRenderer.h"

#include "Core/Log.h"
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
    // GridPass를 _resourceManager보다 먼저 해제합니다.
    // Renderer::Shutdown()이 _resourceManager를 삭제하면 머티리얼 ResourceSet 해제 시
    // vkResetDescriptorPool이 호출되어 같은 풀의 GridSet까지 무효화됩니다.
    _gridPass.reset();
    Renderer::Shutdown();
}

void ForwardRenderer::Render(Scene* scene, RenderTarget* renderTarget)
{
    RenderSceneSnapshot sceneSnapshot = _resourceManager->BuildRenderSceneSnapshot(scene, _shaderLibrary);
    Render(sceneSnapshot, renderTarget);
}

void ForwardRenderer::Render(const RenderSceneSnapshot& snapshot, RenderTarget* renderTarget)
{
    _currentRenderTarget = renderTarget;
    const RenderTargetInfo& rtInfo = _currentRenderTarget->GetInfo();

    // RHIRenderPass 생성을 위한 RenderPassInfo 구성 (해시 캐시 키로 사용됨)
    RenderPassInfo renderPassInfo = {};
    renderPassInfo.colorAttachmentCount = 1;
    Attachment ca{};
    ca.format         = rtInfo.colorTextureInfos[0].format;
    ca.clearValue     = ClearValue(0.33f, 0.33f, 0.33f, 1.0f);
    ca.isDepthStencil = false;
    ca.loadAction     = ELoadAction::Clear;
    ca.storeAction    = EStoreAction::Store;
    renderPassInfo.colorAttachments.push_back(ca);

    if (rtInfo.useDepthStencilTexture)
    {
        Attachment dsa{};
        dsa.format                               = rtInfo.depthStencilInfo.format;
        dsa.clearValue                           = ClearValue(1.0f, 0.0f);
        dsa.isDepthStencil                       = true;
        dsa.loadAction                           = ELoadAction::Clear;
        dsa.storeAction                          = EStoreAction::Store;
        renderPassInfo.depthStencilAttachment    = dsa;
        renderPassInfo.useDepthStencilAttachment = true;
    }
    renderPassInfo.isSwapchainRenderPass = false;

    SceneResource sceneResource = _resourceManager->BuildSceneResource(snapshot);

    auto makeRenderingInfo = [&](const RenderPassInfo& sourceInfo) -> RenderingInfo
    {
        RenderingInfo renderingInfo{};
        renderingInfo.colorAttachmentCount = sourceInfo.colorAttachmentCount;
        renderingInfo.useDepthStencilAttachment = sourceInfo.useDepthStencilAttachment;
        renderingInfo.isSwapchainRendering = sourceInfo.isSwapchainRenderPass;
        renderingInfo.renderArea = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
        renderingInfo.enableAutomaticTransitions = false;

        renderingInfo.colorAttachments.reserve(sourceInfo.colorAttachments.size());
        for (uint32 i = 0; i < sourceInfo.colorAttachmentCount; i++)
        {
            RenderingAttachmentInfo attachmentInfo{};
            attachmentInfo.texture = _currentRenderTarget->GetColorTexture(i);
            attachmentInfo.attachment = sourceInfo.colorAttachments[i];
            renderingInfo.colorAttachments.push_back(attachmentInfo);
        }

        if (sourceInfo.useDepthStencilAttachment)
        {
            renderingInfo.depthStencilAttachment.texture = _currentRenderTarget->GetDepthStencilTexture();
            renderingInfo.depthStencilAttachment.attachment = sourceInfo.depthStencilAttachment;
        }

        return renderingInfo;
    };

    RenderingInfo opaqueRenderingInfo = makeRenderingInfo(renderPassInfo);
    PipelineRenderTargetLayout opaqueRenderTargetLayout = opaqueRenderingInfo.ToRenderTargetLayout();

    // ------------------------------------------------------------------
    // Grid Pass lazy init
    // ------------------------------------------------------------------
    if (!_gridPass)
    {
        _gridPass = MakeScoped<ForwardGridPass>();
        if (!_gridPass->Initialize(_shaderLibrary, _rhiContext))
        {
            HS_LOG(warning, "[ForwardRenderer] GridPass 초기화 실패 — Grid.slang 확인 필요");
        }
    }

    // Grid Pass용 RenderPassInfo — Opaque 결과를 이어받으므로 loadAction=Load
    RenderPassInfo gridPassInfo{};
    gridPassInfo.colorAttachmentCount = 1;
    Attachment gca{};
    gca.format         = rtInfo.colorTextureInfos[0].format;
    gca.clearValue     = ClearValue(0.0f, 0.0f, 0.0f, 0.0f);
    gca.isDepthStencil = false;
    gca.loadAction     = ELoadAction::Load;
    gca.storeAction    = EStoreAction::Store;
    gridPassInfo.colorAttachments.push_back(gca);

    if (rtInfo.useDepthStencilTexture)
    {
        Attachment gdsa{};
        gdsa.format                            = rtInfo.depthStencilInfo.format;
        gdsa.clearValue                        = ClearValue(1.0f, 0.0f);
        gdsa.isDepthStencil                    = true;
        gdsa.loadAction                        = ELoadAction::Load;
        gdsa.storeAction                       = EStoreAction::Store;
        gridPassInfo.depthStencilAttachment    = gdsa;
        gridPassInfo.useDepthStencilAttachment = true;
    }
    gridPassInfo.isSwapchainRenderPass = false;

    RenderingInfo gridRenderingInfo = makeRenderingInfo(gridPassInfo);
    PipelineRenderTargetLayout gridRenderTargetLayout = gridRenderingInfo.ToRenderTargetLayout();

    // ------------------------------------------------------------------
    // RenderGraph 프레임 시작
    // ------------------------------------------------------------------
    _graphBuilder.Setup(_curCommandBuffer);

    struct OpaquePassParameters
    {
        // DepthMap, ShadowMap 등 추후 추가
    } opaqueParams;

    _graphBuilder.AddPass("Opaque", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &opaqueParams,

        // Setup 람다: 이 패스가 읽고 쓰는 리소스를 선언합니다.
        [&](RenderGraphBuilder& builder, RGPass* pass, OpaquePassParameters* passParams) -> void
        {
            // 렌더 타겟 텍스처를 외부 리소스로 등록하고 쓰기 의존성을 선언합니다.
            // 배리어 삽입 및 Culling 방지에 사용됩니다.
            RGTexture* colorTex = builder.RegisterExternalTexture(renderTarget->GetColorTexture(0));
            builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

            if (rtInfo.useDepthStencilTexture)
            {
                RGTexture* depthTex = builder.RegisterExternalTexture(renderTarget->GetDepthStencilTexture());
                builder.Write(pass, depthTex, ERGTextureAccess::DepthAttachmentWrite);
            }
        },

        // Execute 람다: 실제 렌더링 커맨드를 기록합니다.
        [&](RHICommandBuffer& commandBuffer) -> void
        {
            RenderResourceManager* resMgr = GetResourceManager();

            float debugColor[4]{0.2f, 0.5f, 0.8f, 1.0f};

            commandBuffer.BeginRendering(opaqueRenderingInfo);
            commandBuffer.PushDebugMark("Opaque Pass", debugColor);
            commandBuffer.SetViewport(Viewport{0.0f, 0.0f,
                static_cast<float>(_currentRenderTarget->GetWidth()),
                static_cast<float>(_currentRenderTarget->GetHeight()), 0.0f, 1.0f});
            commandBuffer.SetScissor(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());

            for (const auto& renderModel : sceneResource.renderModels)
            {
                Material* mat = renderModel.material;
                if (!mat) continue;
                RHIGraphicsPipeline* pipeline = resMgr->GetOrCreatePipeline(mat, opaqueRenderTargetLayout);
                if (!pipeline) continue;

                commandBuffer.BindPipeline(pipeline);
                if (!renderModel.drawResource || !renderModel.drawResource->resourceSet) continue;
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

    struct GridPassParameters
    {
    } gridParams;

    _graphBuilder.AddPass("Grid", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &gridParams,

        // Setup: 의존성 선언
        [&](RenderGraphBuilder& builder, RGPass* pass, GridPassParameters*) -> void
        {
            // color: Write (Grid도 color를 수정)
            RGTexture* colorTex = builder.RegisterExternalTexture(renderTarget->GetColorTexture(0));
            builder.Write(pass, colorTex, ERGTextureAccess::ColorAttachmentWrite);

            // depth: Read — Opaque.Write(depth) → Grid.Read(depth) 의존성으로 실행 순서 보장
            if (rtInfo.useDepthStencilTexture)
            {
                RGTexture* depthTex = builder.RegisterExternalTexture(renderTarget->GetDepthStencilTexture());
                builder.Read(pass, depthTex, ERGTextureAccess::DepthAttachmentRead);
            }
        },

        // Execute: 렌더링 커맨드 기록
        [&](RHICommandBuffer& commandBuffer) -> void
        {
            if (!_gridPass || !_gridPass->IsInitialized()) return;

            RHIBuffer* perViewBuffer = sceneResource.cameraResources.empty()
                ? nullptr : sceneResource.cameraResources[0]->perViewBuffer;

            RHIGraphicsPipeline* pipeline = _gridPass->GetOrCreatePipeline(
                gridRenderTargetLayout, perViewBuffer);
            if (!pipeline) return;

            float debugColor[4]{0.4f, 0.8f, 0.4f, 1.0f};

            commandBuffer.BeginRendering(gridRenderingInfo);
            commandBuffer.PushDebugMark("Grid Pass", debugColor);
            commandBuffer.SetViewport(Viewport{0.0f, 0.0f,
                static_cast<float>(_currentRenderTarget->GetWidth()),
                static_cast<float>(_currentRenderTarget->GetHeight()), 0.0f, 1.0f});
            commandBuffer.SetScissor(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());

            commandBuffer.BindPipeline(pipeline);
            commandBuffer.BindResourceSet(_gridPass->GetResourceSet());
            commandBuffer.DrawArrays(0, 6, 1); // fullscreen triangle, 버텍스 버퍼 없음

            commandBuffer.PopDebugMark();
            commandBuffer.EndRendering();
        });

    _graphBuilder.Compile();
    _graphBuilder.Execute();
    _graphBuilder.Reset();
}

HS_NS_END
