#include "Engine/Renderer/RenderPass/ForwardOpaquePass.h"

#include "Core/Log.h"
#include "Core/SystemContext.h"

#include "Renderer/RenderPath.h"
#include "Renderer/RenderResourceManager.h"
#include "Renderer/ShaderLibrary.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"
#include "RHI/CommandHandle.h"
#include "RHI/RHIContext.h"

#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Model.h"
#include "Resource/Shader.h"
#include "Resource/ResourceDefinition.h"

#include "Engine/Camera.h"

HS_NS_BEGIN

ForwardOpaquePass::ForwardOpaquePass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder)
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
    ca.loadAction     = ELoadAction::CLEAR;
    ca.storeAction    = EStoreAction::STORE;
    _renderPassInfo.colorAttachments.push_back(ca);

    if (rtInfo.useDepthStencilTexture)
    {
        Attachment dsa{};
        dsa.format                                = rtInfo.depthStencilInfo.format;
        dsa.clearValue                            = ClearValue(1.0f, 0.0f);
        dsa.isDepthStencil                        = true;
        dsa.loadAction                            = ELoadAction::CLEAR;
        dsa.storeAction                           = EStoreAction::STORE;
        _renderPassInfo.depthStencilAttachment    = dsa;
        _renderPassInfo.useDepthStencilAttachment = true;
    }

    _renderPassInfo.isSwapchainRenderPass = false;
}

void ForwardOpaquePass::Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass)
{
    // No-op: use the 3-arg version
}

void ForwardOpaquePass::Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass, const RenderParameter& param)
{
    if (param.cameras.empty() || param.models.empty())
    {
        return;
    }

    RenderResourceManager* resMgr = param.resourceManager;
    ShaderLibrary* shaderLib = param.shaderLibrary;

    if (!resMgr)
    {
        HS_LOG(error, "[ForwardOpaquePass] RenderResourceManager is null");
        return;
    }

    // Update camera and fill PerView UBO
    Camera* camera = param.cameras[0];
    camera->Update();

    CameraResource* camRes = resMgr->GetOrCreateCameraResource(camera);
    if (!camRes) return;
    resMgr->SetActiveCameraResource(camRes);

    PerView perViewData{};
    perViewData.viewMatrix                  = camera->GetViewMatrix();
    perViewData.projectionMatrix            = camera->GetProjectionMatrix();
    perViewData.viewProjectionMatrix        = camera->GetViewProjectionMatrix();
    perViewData.inverseViewMatrix           = camera->GetInverseViewMatrix();
    perViewData.inverseProjectionMatrix     = camera->GetInverseProjectionMatrix();
    perViewData.inverseViewProjectionMatrix = camera->GetInverseViewProjectionMatrix();
    perViewData.cameraPosition              = glm::vec4(camera->GetPosition(), 1.0f);

    commandBuffer->UpdateBuffer(camRes->perViewBuffer, 0, &perViewData, sizeof(PerView));

    // Begin render pass
    RHIFramebuffer* framebuffer = _renderer->GetHandleCache()->GetFramebuffer(renderPass, _currentRenderTarget);

    float debugColor[4]{0.2f, 0.5f, 0.8f, 1.0f};
    commandBuffer->PushDebugMark("Opaque Pass", debugColor);

    for (auto* model : param.models)
    {
        Material* mat = model->GetMaterial();
        Mesh* mesh    = model->GetMesh();
        if (!mat || !mesh) continue;

        // Assign default shader if material has no compiled shader
        Shader* shader = mat->GetShader();
        if ((!shader || !shader->IsCompiledEx()) && shaderLib)
        {
            Shader* defaultShader = shaderLib->GetOrCompile("BlinnPhong");
            if (defaultShader)
            {
                mat->SetShader(defaultShader);
                shader = defaultShader;
            }
        }

        if (!shader || !shader->IsCompiledEx()) continue;

        const ShaderReflectionDataEx& reflection = shader->GetReflection();

        // Get or create model resource (PerDraw UBO)
        ModelResource* modelRes = resMgr->GetOrCreateModelResource(model);
        if (!modelRes) continue;
        resMgr->SetActiveModelResource(modelRes);

        // Update PerDraw
        PerDraw perDrawData{};
        perDrawData.modelMatrix        = model->GetWorldMatrix();
        perDrawData.inverseModelMatrix = model->GetInverseWorldMatrix();
        commandBuffer->UpdateBuffer(modelRes->perDrawBuffer, 0, &perDrawData, sizeof(PerDraw));

        // Get or create all resources from the manager
        MaterialResource* matRes = resMgr->GetOrCreateMaterialResources(mat);
        if (!matRes) continue;

        MeshResource* meshRes = resMgr->GetOrCreateMeshResources(mesh, reflection);
        if (!meshRes) continue;

        RHIGraphicsPipeline* pipeline = resMgr->GetOrCreatePipeline(mat, renderPass);
        if (!pipeline) continue;

        Area area = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
        commandBuffer->BeginRenderPass(renderPass, framebuffer, area);
        commandBuffer->SetViewport(Viewport{0.0f, 0.0f, static_cast<float>(framebuffer->info.width), static_cast<float>(framebuffer->info.height), 0.0f, 1.0f});
        commandBuffer->SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);

        // Bind and draw
        commandBuffer->BindPipeline(pipeline);
        commandBuffer->BindResourceSet(matRes->resourceSet);

        uint32 vbOffset     = 0;
        const RHIBuffer* vb = meshRes->vertexBuffer;
        commandBuffer->BindVertexBuffers(&vb, &vbOffset, 1);
        commandBuffer->BindIndexBuffer(meshRes->indexBuffer);
        commandBuffer->DrawIndexed(0, meshRes->indexCount, 1, 0);
    }

    commandBuffer->EndRenderPass();
    commandBuffer->PopDebugMark();
}

void ForwardOpaquePass::OnAfterRendering()
{
}

HS_NS_END
