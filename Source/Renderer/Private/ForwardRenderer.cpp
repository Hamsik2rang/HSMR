#include "Renderer/ForwardRenderer.h"

#include "RHI/RHIContext.h"

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
    //...
    const RenderTargetInfo& rtInfo = _currentRenderTarget->GetInfo();

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

    // TODO: Dynamic Rendering이 들어가면 제거될 수 있는 부분(RHIRenderPass, RHIFramebuffer)
    RHIRenderPass* renderPass = GetHandleCache()->GetRenderPass(renderPassInfo);

#pragma region>>> Opaque Pass
    // Opaque Pass
    struct OpaquePassParameters
    {
        //...
        // DepthMap, ShadowMap 등이 들어갈 것
    } opaqueParams;

    ERGPassFlag opaquePasFlag = ERGPassFlag::Raster | ERGPassFlag::NeverCull;

    _graphBuilder.AddPass("Opaque", opaquePasFlag, & opaqueParams,
        [&](RenderGraphBuilder& builder, RGPass* pass, OpaquePassParameters* passParams) -> void
    {
        //...
        return;
    },
        [&](RHICommandBuffer& commandBuffer) -> void
    {
        RenderResourceManager* resMgr = GetResourceManager();

        RHIFramebuffer* framebuffer = GetHandleCache()->GetFramebuffer(renderPass, _currentRenderTarget);

        float debugColor[4]{0.2f, 0.5f, 0.8f, 1.0f};
        commandBuffer.PushDebugMark("Opaque Pass", debugColor);

        Area area = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());

        // Always begin the render pass to ensure proper image layout transitions and clear
        commandBuffer.BeginRenderPass(renderPass, framebuffer, area);
        commandBuffer.SetViewport(Viewport{0.0f, 0.0f,
            static_cast<float>(framebuffer->info.width),
            static_cast<float>(framebuffer->info.height), 0.0f, 1.0f});
        commandBuffer.SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);

        for (const auto& renderModel : sceneResource.renderModels)
        {
            // Get pipeline (pass-specific, can't be pre-resolved)
            Material* mat = renderModel.material;
            if (!mat) continue;
            RHIGraphicsPipeline* pipeline = resMgr->GetOrCreatePipeline(mat, renderPass);
            if (!pipeline) continue;

            // Bind and draw using pre-resolved resources
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
#pragma endregion

}

HS_NS_END
