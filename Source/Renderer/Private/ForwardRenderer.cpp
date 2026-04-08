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
    _debugPass.reset();
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

    // Dynamic rendering info is the single pass description for modern and legacy RHI paths.
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
    Attachment ca{};
    ca.format         = rtInfo.colorTextureInfos[0].format;
    ca.clearValue     = ClearValue(0.33f, 0.33f, 0.33f, 1.0f);
    ca.isDepthStencil = false;
    ca.loadAction     = ELoadAction::Clear;
    ca.storeAction    = EStoreAction::Store;
    Attachment dsa{};
    bool useDepthStencilAttachment = false;
    if (rtInfo.useDepthStencilTexture)
    {
        dsa.format                               = rtInfo.depthStencilInfo.format;
        dsa.clearValue                           = ClearValue(1.0f, 0.0f);
        dsa.isDepthStencil                       = true;
        dsa.loadAction                           = ELoadAction::Clear;
        dsa.storeAction                          = EStoreAction::Store;
        useDepthStencilAttachment                = true;
    }

    SceneResource sceneResource = _resourceManager->BuildSceneResource(snapshot);

    RenderingInfo opaqueRenderingInfo = makeRenderingInfo(ca, useDepthStencilAttachment, dsa);
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

    if (_debugPassEnabled && !_debugPass)
    {
        _debugPass = MakeScoped<ForwardDebugPass>();
        if (!_debugPass->Initialize(_shaderLibrary, _rhiContext))
        {
            HS_LOG(warning, "[ForwardRenderer] DebugPass initialization failed. Check DebugLine.slang.");
        }
    }

    // Grid pass loads the opaque result.
    Attachment gca{};
    gca.format         = rtInfo.colorTextureInfos[0].format;
    gca.clearValue     = ClearValue(0.0f, 0.0f, 0.0f, 0.0f);
    gca.isDepthStencil = false;
    gca.loadAction     = ELoadAction::Load;
    gca.storeAction    = EStoreAction::Store;
    Attachment gdsa{};
    bool useGridDepthStencilAttachment = false;
    if (rtInfo.useDepthStencilTexture)
    {
        gdsa.format                            = rtInfo.depthStencilInfo.format;
        gdsa.clearValue                        = ClearValue(1.0f, 0.0f);
        gdsa.isDepthStencil                    = true;
        gdsa.loadAction                        = ELoadAction::Load;
        gdsa.storeAction                       = EStoreAction::Store;
        useGridDepthStencilAttachment          = true;
    }

    RenderingInfo gridRenderingInfo = makeRenderingInfo(gca, useGridDepthStencilAttachment, gdsa);
    PipelineRenderTargetLayout gridRenderTargetLayout = gridRenderingInfo.ToRenderTargetLayout();
    RenderingInfo debugRenderingInfo = gridRenderingInfo;
    PipelineRenderTargetLayout debugRenderTargetLayout = debugRenderingInfo.ToRenderTargetLayout();
    bool hasDebugDrawData = false;
    if (_debugPassEnabled && _debugPass && _debugPass->IsInitialized())
    {
        hasDebugDrawData = _debugPass->Prepare(snapshot) && _debugPass->HasDrawData();
    }

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

    if (_debugPassEnabled && hasDebugDrawData)
    {
        struct DebugPassParameters
        {
        } debugParams;

        _graphBuilder.AddPass("Debug", ERGPassFlag::Raster | ERGPassFlag::NeverCull, &debugParams,

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

            [&](RHICommandBuffer& commandBuffer) -> void
            {
                if (!_debugPassEnabled || !_debugPass || !_debugPass->IsInitialized() || !hasDebugDrawData) return;

                RHIBuffer* perViewBuffer = sceneResource.cameraResources.empty()
                    ? nullptr : sceneResource.cameraResources[0]->perViewBuffer;

                RHIGraphicsPipeline* pipeline = _debugPass->GetOrCreatePipeline(
                    debugRenderTargetLayout, perViewBuffer);
                if (!pipeline || !_debugPass->GetResourceSet() || !_debugPass->GetVertexBuffer()) return;

                float debugColor[4]{1.0f, 0.8f, 0.2f, 1.0f};

                commandBuffer.BeginRendering(debugRenderingInfo);
                commandBuffer.PushDebugMark("Debug Pass", debugColor);
                commandBuffer.SetViewport(Viewport{0.0f, 0.0f,
                    static_cast<float>(_currentRenderTarget->GetWidth()),
                    static_cast<float>(_currentRenderTarget->GetHeight()), 0.0f, 1.0f});
                commandBuffer.SetScissor(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());

                commandBuffer.BindPipeline(pipeline);
                commandBuffer.BindResourceSet(_debugPass->GetResourceSet());

                uint32 vbOffset = 0;
                const RHIBuffer* vertexBuffer = _debugPass->GetVertexBuffer();
                commandBuffer.BindVertexBuffers(&vertexBuffer, &vbOffset, 1);
                commandBuffer.DrawArrays(0, _debugPass->GetVertexCount(), 1);

                commandBuffer.PopDebugMark();
                commandBuffer.EndRendering();
            });
    }

    _graphBuilder.Compile();
    _graphBuilder.Execute();
    _graphBuilder.Reset();
}

HS_NS_END
