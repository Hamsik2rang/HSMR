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

    _renderingInfo = {};
    _renderingInfo.colorAttachmentCount = 1;
    _renderingInfo.renderArea = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
    _renderingInfo.enableAutomaticTransitions = false;

    Attachment ca{};
    ca.format         = rtInfo.colorTextureInfos[0].format;
    ca.clearValue     = ClearValue(0.33f, 0.33f, 0.33f, 1.0f);
    ca.isDepthStencil = false;
    ca.loadAction     = ELoadAction::Clear;
    ca.storeAction    = EStoreAction::Store;

    RenderingAttachmentInfo colorAttachment{};
    colorAttachment.texture = _currentRenderTarget->GetColorTexture(0);
    colorAttachment.attachment = ca;
    _renderingInfo.colorAttachments.push_back(colorAttachment);

    if (rtInfo.useDepthStencilTexture)
    {
        Attachment dsa{};
        dsa.format                                = rtInfo.depthStencilInfo.format;
        dsa.clearValue                            = ClearValue(1.0f, 0.0f);
        dsa.isDepthStencil                        = true;
        dsa.loadAction                            = ELoadAction::Clear;
        dsa.storeAction                           = EStoreAction::Store;
        _renderingInfo.depthStencilAttachment.texture = _currentRenderTarget->GetDepthStencilTexture();
        _renderingInfo.depthStencilAttachment.attachment = dsa;
        _renderingInfo.useDepthStencilAttachment = true;
    }
}

void ForwardOpaquePass::Execute(RHICommandBuffer* commandBuffer, const SceneResource& sceneResource)
{
    RenderResourceManager* resMgr = _renderer->GetResourceManager();

    float debugColor[4]{0.2f, 0.5f, 0.8f, 1.0f};
    commandBuffer->PushDebugMark("Opaque Pass", debugColor);

    commandBuffer->BeginRendering(_renderingInfo);
    commandBuffer->SetViewport(Viewport{0.0f, 0.0f,
        static_cast<float>(_currentRenderTarget->GetWidth()),
        static_cast<float>(_currentRenderTarget->GetHeight()), 0.0f, 1.0f});
    commandBuffer->SetScissor(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());

    for (const auto& renderModel : sceneResource.renderModels)
    {
        // Get pipeline (pass-specific, can't be pre-resolved)
        Material* mat = renderModel.material;
        if (!mat) continue;
        RHIGraphicsPipeline* pipeline = resMgr->GetOrCreatePipeline(mat, _renderingInfo.ToRenderTargetLayout());
        if (!pipeline) continue;

        // Bind and draw using pre-resolved resources
        commandBuffer->BindPipeline(pipeline);
        commandBuffer->BindResourceSet(renderModel.materialResource->resourceSet);

        uint32 vbOffset     = 0;
        const RHIBuffer* vb = renderModel.meshResource->vertexBuffer;
        commandBuffer->BindVertexBuffers(&vb, &vbOffset, 1);
        commandBuffer->BindIndexBuffer(renderModel.meshResource->indexBuffer);
        commandBuffer->DrawIndexed(0, renderModel.meshResource->indexCount, 1, 0);
    }

    commandBuffer->EndRendering();
    commandBuffer->PopDebugMark();
}

void ForwardOpaquePass::OnAfterRendering()
{
}

HS_NS_END
