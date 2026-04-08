#include "Renderer/RenderPass/ForwardOpaquePass.h"

#include "Core/Log.h"

#include "Renderer/Renderer.h"
#include "Renderer/RenderResourceManager.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"
#include "RHI/CommandHandle.h"
#include "RHI/RHIContext.h"

#include "Resource/Material.h"

HS_NS_BEGIN

ForwardOpaquePass::ForwardOpaquePass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder)
    : ForwardRenderPass(name, renderer, renderingOrder)
{
}

ForwardOpaquePass::~ForwardOpaquePass()
{}

void ForwardOpaquePass::OnBeforeRendering(uint32_t frameIndex)
{
    this->frameIndex = frameIndex;
}

void ForwardOpaquePass::Configure(RenderTarget* renderTarget)
{
    _currentRenderTarget = renderTarget;

    const RenderTargetInfo& rtInfo = _currentRenderTarget->GetInfo();

    _renderPassInfo = {};

    _renderPassInfo.colorAttachmentCount = 1;
    Attachment ca{};
    ca.format         = rtInfo.colorTextureInfos[0].format;
    ca.clearValue     = ClearValue(0.33f, 0.33f, 0.33f, 1.0f);
    ca.isDepthStencil = false;
    ca.loadAction     = ELoadAction::Clear;
    ca.storeAction    = EStoreAction::Store;
    _renderPassInfo.colorAttachments.push_back(ca);

    if (rtInfo.useDepthStencilTexture)
    {
        Attachment dsa{};
        dsa.format                                = rtInfo.depthStencilInfo.format;
        dsa.clearValue                            = ClearValue(1.0f, 0.0f);
        dsa.isDepthStencil                        = true;
        dsa.loadAction                            = ELoadAction::Clear;
        dsa.storeAction                           = EStoreAction::Store;
        _renderPassInfo.depthStencilAttachment    = dsa;
        _renderPassInfo.useDepthStencilAttachment = true;
    }

    _renderPassInfo.isSwapchainRenderPass = false;
}

void ForwardOpaquePass::Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass, const SceneResource& sceneResource)
{
    RenderResourceManager* resMgr = _renderer->GetResourceManager();

    RHIFramebuffer* framebuffer = _renderer->GetHandleCache()->GetFramebuffer(renderPass, _currentRenderTarget);

    float debugColor[4]{0.2f, 0.5f, 0.8f, 1.0f};
    commandBuffer->PushDebugMark("Opaque Pass", debugColor);

    Area area = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());

    // Always begin the render pass to ensure proper image layout transitions and clear
    commandBuffer->BeginRenderPass(renderPass, framebuffer, area);
    commandBuffer->SetViewport(Viewport{0.0f, 0.0f,
        static_cast<float>(framebuffer->info.width),
        static_cast<float>(framebuffer->info.height), 0.0f, 1.0f});
    commandBuffer->SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);

    for (const auto& renderModel : sceneResource.renderModels)
    {
        // Get pipeline (pass-specific, can't be pre-resolved)
        Material* mat = renderModel.material;
        if (!mat) continue;
        RHIGraphicsPipeline* pipeline = resMgr->GetOrCreatePipeline(mat, renderPass);
        if (!pipeline) continue;

        // Bind and draw using pre-resolved resources
        commandBuffer->BindPipeline(pipeline);
        if (!renderModel.drawResource || !renderModel.drawResource->resourceSet) continue;
        commandBuffer->BindResourceSet(renderModel.drawResource->resourceSet);

        uint32 vbOffset     = 0;
        const RHIBuffer* vb = renderModel.meshResource->vertexBuffer;
        commandBuffer->BindVertexBuffers(&vb, &vbOffset, 1);
        commandBuffer->BindIndexBuffer(renderModel.meshResource->indexBuffer);
        commandBuffer->DrawIndexed(0, renderModel.meshResource->indexCount, 1, 0);
    }

    commandBuffer->EndRenderPass();
    commandBuffer->PopDebugMark();
}

void ForwardOpaquePass::OnAfterRendering()
{
}

HS_NS_END
