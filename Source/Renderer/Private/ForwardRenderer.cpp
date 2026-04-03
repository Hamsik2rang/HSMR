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

void ForwardRenderer::Render(Scene* scene, RenderTarget* renderTarget)
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

    SceneResource sceneResource = _resourceManager->BuildSceneResource(scene, _shaderLibrary);

    RHIRenderPass* renderPass = GetHandleCache()->GetRenderPass(renderPassInfo);

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
            RHIFramebuffer* framebuffer   = GetHandleCache()->GetFramebuffer(renderPass, _currentRenderTarget);

            float debugColor[4]{0.2f, 0.5f, 0.8f, 1.0f};
            commandBuffer.PushDebugMark("Opaque Pass", debugColor);

            Area area = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());

            commandBuffer.BeginRenderPass(renderPass, framebuffer, area);
            commandBuffer.SetViewport(Viewport{0.0f, 0.0f,
                static_cast<float>(framebuffer->info.width),
                static_cast<float>(framebuffer->info.height), 0.0f, 1.0f});
            commandBuffer.SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);

            for (const auto& renderModel : sceneResource.renderModels)
            {
                Material* mat = renderModel.material;
                if (!mat) continue;
                RHIGraphicsPipeline* pipeline = resMgr->GetOrCreatePipeline(mat, renderPass);
                if (!pipeline) continue;

                commandBuffer.BindPipeline(pipeline);
                commandBuffer.BindResourceSet(renderModel.materialResource->resourceSet);

                uint32 vbOffset     = 0;
                const RHIBuffer* vb = renderModel.meshResource->vertexBuffer;
                commandBuffer.BindVertexBuffers(&vb, &vbOffset, 1);
                commandBuffer.BindIndexBuffer(renderModel.meshResource->indexBuffer);
                commandBuffer.DrawIndexed(0, renderModel.meshResource->indexCount, 1, 0);
            }

            commandBuffer.EndRenderPass();
            commandBuffer.PopDebugMark();
        });

    _graphBuilder.Compile();
    _graphBuilder.Execute();
    _graphBuilder.Reset();
}

HS_NS_END
