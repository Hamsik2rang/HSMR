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

// ECS Scene support
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

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

void ForwardOpaquePass::Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass)
{
    // No-op: use the 3-arg version
}

void ForwardOpaquePass::Execute(RHICommandBuffer* commandBuffer, RHIRenderPass* renderPass, const RenderParameter& param)
{
    bool hasModels = !param.models.empty();
    bool hasScene = param.scene != nullptr;

    if (param.cameras.empty() || (!hasModels && !hasScene))
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

    Area area = Area(0, 0, _currentRenderTarget->GetWidth(), _currentRenderTarget->GetHeight());
    bool renderPassStarted = false;

    // Helper lambda to render a single mesh/material pair with transform
    auto renderMeshMaterial = [&](Mesh* mesh, Material* mat, const glm::mat4& worldMatrix, const glm::mat4& invWorldMatrix, Model* model = nullptr)
    {
        if (!mat || !mesh) return;

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

        if (!shader || !shader->IsCompiledEx()) return;

        const ShaderReflectionDataEx& reflection = shader->GetReflection();

        // Get or create model resource (PerDraw UBO) - use Model if available, otherwise create from entity
        ModelResource* modelRes = nullptr;
        if (model)
        {
            modelRes = resMgr->GetOrCreateModelResource(model);
        }
        else
        {
            // For ECS entities without Model, we need a temporary approach
            // TODO: Create EntityResource similar to ModelResource for ECS
            // For now, skip ECS entities (they won't render until EntityResource is implemented)
            return;
        }

        if (!modelRes) return;
        resMgr->SetActiveModelResource(modelRes);

        // Update PerDraw
        PerDraw perDrawData{};
        perDrawData.modelMatrix        = worldMatrix;
        perDrawData.inverseModelMatrix = invWorldMatrix;
        commandBuffer->UpdateBuffer(modelRes->perDrawBuffer, 0, &perDrawData, sizeof(PerDraw));

        // Get or create all resources from the manager
        MaterialResource* matRes = resMgr->GetOrCreateMaterialResources(mat);
        if (!matRes) return;

        MeshResource* meshRes = resMgr->GetOrCreateMeshResources(mesh, reflection);
        if (!meshRes) return;

        RHIGraphicsPipeline* pipeline = resMgr->GetOrCreatePipeline(mat, renderPass);
        if (!pipeline) return;

        if (!renderPassStarted)
        {
            commandBuffer->BeginRenderPass(renderPass, framebuffer, area);
            commandBuffer->SetViewport(Viewport{0.0f, 0.0f, static_cast<float>(framebuffer->info.width), static_cast<float>(framebuffer->info.height), 0.0f, 1.0f});
            commandBuffer->SetScissor(0, 0, framebuffer->info.width, framebuffer->info.height);
            renderPassStarted = true;
        }

        // Bind and draw
        commandBuffer->BindPipeline(pipeline);
        commandBuffer->BindResourceSet(matRes->resourceSet);

        uint32 vbOffset     = 0;
        const RHIBuffer* vb = meshRes->vertexBuffer;
        commandBuffer->BindVertexBuffers(&vb, &vbOffset, 1);
        commandBuffer->BindIndexBuffer(meshRes->indexBuffer);
        commandBuffer->DrawIndexed(0, meshRes->indexCount, 1, 0);
    };

    // Render legacy Model objects
    for (auto* model : param.models)
    {
        Material* mat = model->GetMaterial();
        Mesh* mesh    = model->GetMesh();
        renderMeshMaterial(mesh, mat, model->GetWorldMatrix(), model->GetInverseWorldMatrix(), model);
    }

    // Render ECS Scene entities with MeshRendererComponent
    if (hasScene)
    {
        auto& registry = param.scene->GetRegistry();
        auto view = registry.view<TransformComponent, MeshRendererComponent>();

        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& meshRenderer = view.get<MeshRendererComponent>(entity);

            if (!meshRenderer.IsValidForRendering())
                continue;

            // Check visibility
            if (!meshRenderer.isVisible)
                continue;

            // Render each submesh with its material
            Mesh* mesh = meshRenderer.mesh;
            uint32 submeshCount = mesh ? 1 : 0; // TODO: Get actual submesh count from Mesh

            for (uint32 i = 0; i < submeshCount; ++i)
            {
                Material* mat = meshRenderer.GetMaterial(i);
                if (!mat) continue;

                const glm::mat4& worldMatrix = transform.worldMatrix;
                glm::mat4 invWorldMatrix = glm::inverse(worldMatrix);

                // TODO: Need EntityResource for proper GPU resource management
                // For now, ECS entities won't render - this is a placeholder
                // renderMeshMaterial(mesh, mat, worldMatrix, invWorldMatrix, nullptr);
            }
        }
    }

    if (renderPassStarted)
    {
        commandBuffer->EndRenderPass();
    }
    commandBuffer->PopDebugMark();
}

void ForwardOpaquePass::OnAfterRendering()
{
}

HS_NS_END
