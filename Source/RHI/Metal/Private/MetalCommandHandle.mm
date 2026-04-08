#include "RHI/Metal/MetalCommandHandle.h"
#include "RHI/Metal/MetalDefinition.h"

#include "RHI/Metal/MetalContext.h"
#include "RHI/Metal/MetalResourceHandle.h"
#include "RHI/Metal/MetalRenderHandle.h"

#include "Core/Log.h"

#include <vector>

HS_NS_BEGIN

static uint32 resolveStageBinding(const ResourceBinding& binding, EShaderStage stage)
{
    const uint32 resolved = binding.nativeBindingSlots.GetStageBindingOr(binding.binding, stage);
    if (!binding.nativeBindingSlots.HasStageBinding(stage))
    {
//        HS_LOG(debug,
//               "[MetalCommandBuffer] Fallback to logical binding %u for resource '%s' on stage %u",
//               static_cast<uint32>(binding.binding),
//               binding.name.empty() ? "(unnamed)" : binding.name.c_str(),
//               static_cast<uint32>(stage));
    }
    return resolved;
}

//
// id<MTLCommandBuffer>        handle;
// id<MTLRenderCommandEncoder> curRenderEncoder;
// MTLRenderPassDescriptor*    curRenderPassDesc;
// MetalRenderPass*            curBindRenderPass;
// MetalFramebuffer*           curBindFramebuffer;
// MetalGraphicsPipeline*      curBindPipeline;
// MetalBuffer*                curBindIndexBuffer;

MetalCommandPool::MetalCommandPool(const char* name)
    : RHICommandPool(name)
{
}

MetalCommandBuffer::MetalCommandBuffer(const char* name, id<MTLDevice> device, id<MTLCommandQueue> commandQueue)
    : RHICommandBuffer(name)
    , device(device)
    , cmdQueue(commandQueue)
    , handle(nil)
    , curRenderEncoder(nil)
    , curComputeEncoder(nil)
    , curRenderPassDesc(nil)
    , curBindPipeline(nullptr)
    , curBindComputePipeline(nullptr)
    , curBindIndexBuffer(nullptr)
{
}

MetalCommandBuffer::~MetalCommandBuffer()
{
}

void MetalCommandBuffer::Begin()
{
    HS_ASSERT(!_isBegan, "CommandBuffer is already began");

    curRenderEncoder      = nil;
    curComputeEncoder     = nil;
    curRenderPassDesc     = nil;
    curBindPipeline       = nullptr;
    curBindComputePipeline = nullptr;
    curBindIndexBuffer    = nullptr;

    handle = [cmdQueue commandBufferWithUnretainedReferences];

    _isBegan           = true;
    _isRenderPassBegan = false;
}

void MetalCommandBuffer::End()
{
    HS_ASSERT(_isBegan, "Commandbuffer isn't began yet");

    Reset();
}

void MetalCommandBuffer::Reset()
{
    if (nil != curRenderEncoder)
    {
        [curRenderEncoder endEncoding];
        curRenderEncoder = nil;
    }

    if (nil != curComputeEncoder)
    {
        [curComputeEncoder endEncoding];
        curComputeEncoder = nil;
    }

    _isBegan = false;
}

void MetalCommandBuffer::BindPipeline(RHIGraphicsPipeline* pipeline)
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");
    HS_CHECK(_isRenderPassBegan, "RenderPass isn't began yet");
    HS_CHECK(pipeline, "Pipeline is null");
    curBindPipeline = static_cast<MetalGraphicsPipeline*>(pipeline);

    [curRenderEncoder setRenderPipelineState:curBindPipeline->pipelineState];
    if (pipeline->info.renderTargetLayout.useDepthStencilAttachment)
    {
        [curRenderEncoder setDepthStencilState:curBindPipeline->depthStencilState];
    }
    [curRenderEncoder setFrontFacingWinding:MetalUtility::ToWinding(pipeline->info.rasterizerDesc.frontFace)];
    [curRenderEncoder setCullMode:MetalUtility::ToCullMode(pipeline->info.rasterizerDesc.cullMode)];
    [curRenderEncoder setTriangleFillMode:MetalUtility::ToPolygonMode(pipeline->info.rasterizerDesc.polygonMode)];

    curBindPipeline = static_cast<MetalGraphicsPipeline*>(pipeline);
}

void MetalCommandBuffer::BindResourceSet(RHIResourceSet* rSet)
{
    for (size_t i = 0; i < rSet->layouts.size(); i++)
    {
        const auto& bindings = rSet->layouts[i]->bindings;
        for (size_t j = 0; j < bindings.size(); j++)
        {
            const ResourceBinding& rb = bindings[j];

            switch (rb.type)
            {
                case EResourceType::UniformBuffer:
                {
                    bindBuffers(rb);
                }
                break;
                case EResourceType::CombinedImageSampler:
                case EResourceType::SampledImage:
                {
                    bindTextures(rb);
                    if (rb.type == EResourceType::CombinedImageSampler && !rb.resource.samplers.empty())
                    {
                        bindSamplers(rb);
                    }
                }
                break;
                case EResourceType::Sampler:
                {
                    bindSamplers(rb);
                }
                break;
                default:
                {
                    HS_LOG(crash, "Not Implemented ResourceType");
                    break;
                }
            }
        }
    }
}

void MetalCommandBuffer::SetViewport(const Viewport& viewport)
{
    [curRenderEncoder setViewport:MetalUtility::ToViewport(viewport)];
}

void MetalCommandBuffer::SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height)
{
    MTLScissorRect rect = {x, y, width, height};
    [curRenderEncoder setScissorRect:rect];
}

void MetalCommandBuffer::BindIndexBuffer(RHIBuffer* indexBuffer)
{
    curBindIndexBuffer = static_cast<MetalBuffer*>(indexBuffer);
}

void MetalCommandBuffer::BindVertexBuffers(const RHIBuffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount)
{
    for (uint8 i = 0; i < bufferCount; i++)
    {
        auto vertexBuffer = static_cast<const MetalBuffer*>(vertexBuffers[i]);

        [curRenderEncoder setVertexBuffer:vertexBuffer->handle offset:offsets[i] atIndex:kMetalVertexBufferBaseIndex + i];
    }
}

void MetalCommandBuffer::DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount)
{
    MTLPrimitiveType primType = MetalUtility::ToPrimitiveTopology(curBindPipeline->info.inputAssemblyDesc.primitiveTopology);

    [curRenderEncoder drawPrimitives:primType
                         vertexStart:firstVertex
                         vertexCount:vertexCount
                       instanceCount:instanceCount
                        baseInstance:0];
}
void MetalCommandBuffer::DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset)
{
    MTLPrimitiveType primType = MetalUtility::ToPrimitiveTopology(curBindPipeline->info.inputAssemblyDesc.primitiveTopology);

    [curRenderEncoder drawIndexedPrimitives:primType
                                 indexCount:indexCount
                                  indexType:MTLIndexTypeUInt32
                                indexBuffer:curBindIndexBuffer->handle
                          indexBufferOffset:firstIndex
                              instanceCount:instanceCount
                                 baseVertex:vertexOffset
                               baseInstance:0];
}

void MetalCommandBuffer::EndRendering()
{
    if (nil != curRenderEncoder)
    {
        [curRenderEncoder endEncoding];
        curRenderEncoder = nil;
    }

    curRenderPassDesc  = nil;
    curBindPipeline    = nullptr;

    _isRenderPassBegan = false;
}

void MetalCommandBuffer::BeginRendering(const RenderingInfo& renderingInfo)
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");

    curRenderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    for (uint32 i = 0; i < renderingInfo.colorAttachmentCount; i++)
    {
        const RenderingAttachmentInfo& attachmentInfo = renderingInfo.colorAttachments[i];
        MetalTexture* texture = static_cast<MetalTexture*>(attachmentInfo.texture);
        curRenderPassDesc.colorAttachments[i].texture = texture->handle;
        curRenderPassDesc.colorAttachments[i].loadAction = MetalUtility::ToLoadAction(attachmentInfo.attachment.loadAction);
        curRenderPassDesc.colorAttachments[i].storeAction = MetalUtility::ToStoreAction(attachmentInfo.attachment.storeAction);
        curRenderPassDesc.colorAttachments[i].clearColor = MetalUtility::ToClearColor(attachmentInfo.attachment.clearValue.color);
    }

    if (renderingInfo.useDepthStencilAttachment)
    {
        const RenderingAttachmentInfo& attachmentInfo = renderingInfo.depthStencilAttachment;
        MetalTexture* texture = static_cast<MetalTexture*>(attachmentInfo.texture);
        curRenderPassDesc.depthAttachment.texture = texture->handle;
        curRenderPassDesc.depthAttachment.loadAction = MetalUtility::ToLoadAction(attachmentInfo.attachment.loadAction);
        curRenderPassDesc.depthAttachment.storeAction = MetalUtility::ToStoreAction(attachmentInfo.attachment.storeAction);
        curRenderPassDesc.depthAttachment.clearDepth = static_cast<double>(attachmentInfo.attachment.clearValue.depthStencil.depth);
    }

    if (nil != curRenderEncoder)
    {
        [curRenderEncoder endEncoding];
    }

    curRenderEncoder = [handle renderCommandEncoderWithDescriptor:curRenderPassDesc];
    _isRenderPassBegan = true;
}

void MetalCommandBuffer::BindComputePipeline(RHIComputePipeline* pipeline)
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");
    HS_CHECK(pipeline, "Compute Pipeline is null");

    // End render encoder if active (can't have both at once)
    if (nil != curRenderEncoder)
    {
        [curRenderEncoder endEncoding];
        curRenderEncoder = nil;
        _isRenderPassBegan = false;
    }

    // Create compute encoder if not already created
    if (nil == curComputeEncoder)
    {
        curComputeEncoder = [handle computeCommandEncoder];
    }

    curBindComputePipeline = static_cast<MetalComputePipeline*>(pipeline);
    [curComputeEncoder setComputePipelineState:curBindComputePipeline->pipelineState];
}

void MetalCommandBuffer::BindComputeResourceSet(RHIResourceSet* rSet)
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");
    HS_CHECK(curComputeEncoder, "Compute encoder is not active");

    for (size_t i = 0; i < rSet->layouts.size(); i++)
    {
        const auto& bindings = rSet->layouts[i]->bindings;
        for (size_t j = 0; j < bindings.size(); j++)
        {
            const ResourceBinding& rb = bindings[j];

            switch (rb.type)
            {
                case EResourceType::UniformBuffer:
                case EResourceType::StorageBuffer:
                {
                    for (uint8 k = 0; k < rb.arrayCount; k++)
                    {
                        MetalBuffer* buffer = static_cast<MetalBuffer*>(rb.resource.buffers[k]);
                        const uint32 bindingIndex = resolveStageBinding(rb, EShaderStage::Compute);
                        [curComputeEncoder setBuffer:buffer->handle offset:rb.resource.offsets[k] atIndex:bindingIndex + k];
                    }
                }
                break;
                case EResourceType::CombinedImageSampler:
                case EResourceType::SampledImage:
                case EResourceType::StorageImage:
                {
                    for (uint8 k = 0; k < rb.arrayCount; k++)
                    {
                        MetalTexture* texture = static_cast<MetalTexture*>(rb.resource.textures[k]);
                        const uint32 bindingIndex = resolveStageBinding(rb, EShaderStage::Compute);
                        [curComputeEncoder setTexture:texture->handle atIndex:bindingIndex + k];
                    }
                }
                break;
                case EResourceType::Sampler:
                {
                    for (uint8 k = 0; k < rb.arrayCount; k++)
                    {
                        MetalSampler* sampler = static_cast<MetalSampler*>(rb.resource.samplers[k]);
                        const uint32 bindingIndex = resolveStageBinding(rb, EShaderStage::Compute);
                        [curComputeEncoder setSamplerState:sampler->handle atIndex:bindingIndex + k];
                    }
                }
                break;
                default:
                {
                    HS_LOG(crash, "Not Implemented ResourceType for Compute");
                    break;
                }
            }
        }
    }
}

void MetalCommandBuffer::Dispatch(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ)
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");
    HS_CHECK(curComputeEncoder, "Compute encoder is not active");
    HS_CHECK(curBindComputePipeline, "Compute pipeline is not bound");

    MTLSize threadgroupsPerGrid = MTLSizeMake(groupCountX, groupCountY, groupCountZ);

    // Get the max threads per threadgroup from the pipeline state
    NSUInteger maxTotalThreadsPerThreadgroup = curBindComputePipeline->pipelineState.maxTotalThreadsPerThreadgroup;
    NSUInteger threadExecutionWidth = curBindComputePipeline->pipelineState.threadExecutionWidth;

    // Calculate optimal threadgroup size
    // For now, use a simple approach: try to use 256 threads per threadgroup
    NSUInteger threadsPerThreadgroup = MIN(256, maxTotalThreadsPerThreadgroup);
    MTLSize threadgroupSize = MTLSizeMake(threadsPerThreadgroup, 1, 1);

    [curComputeEncoder dispatchThreadgroups:threadgroupsPerGrid threadsPerThreadgroup:threadgroupSize];
}

void MetalCommandBuffer::EndComputePass()
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");

    if (nil != curComputeEncoder)
    {
        [curComputeEncoder endEncoding];
        curComputeEncoder = nil;
    }

    curBindComputePipeline = nullptr;
    _isComputeBegan = false;
}

void MetalCommandBuffer::TextureBarrier(const RHITextureBarrierDesc* barriers, uint32 count)
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");
    (void)barriers;
    (void)count;

    // For Metal, we need to use a blit encoder to synchronize texture access
    // between compute and render passes. This is handled automatically by
    // ending the compute encoder before starting a render pass.
    //
    // For explicit synchronization between compute dispatches, we can use
    // memoryBarrierWithScope on the compute encoder if it's still active.
    if (nil != curComputeEncoder)
    {
        // Use memory barrier for compute shader synchronization
        [curComputeEncoder memoryBarrierWithScope:MTLBarrierScopeTextures];
    }
}

void MetalCommandBuffer::TextureBarrier(RHITexture* texture)
{
    RHITextureBarrierDesc barrier{};
    barrier.texture = texture;
    barrier.before = ERHITextureState::StorageReadWrite;
    barrier.after = ERHITextureState::ShaderRead;
    TextureBarrier(&barrier, 1);
}

void MetalCommandBuffer::CopyTexture(RHITexture* srcTexture, RHITexture* dstTexture)
{
}
void MetalCommandBuffer::UpdateBuffer(RHIBuffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize)
{
    MetalBuffer* mtlBuffer = static_cast<MetalBuffer*>(buffer);
    memcpy(static_cast<uint8_t*>([mtlBuffer->handle contents]) + dstOffset, srcData, dataSize);
}

void MetalCommandBuffer::PushDebugMark(const char* label, float* color)
{
    HS_ASSERT(_isBegan, "CommandBuffer isn't began yet");

    NSString* labelStr = [NSString stringWithUTF8String:label];
    [curRenderEncoder pushDebugGroup:labelStr];
}

void MetalCommandBuffer::PopDebugMark()
{
    [curRenderEncoder popDebugGroup];
}

void MetalCommandBuffer::bindBuffers(const ResourceBinding& binding)
{
    MetalBuffer* const* metalBuffers = reinterpret_cast<MetalBuffer* const*>(binding.resource.buffers.data());
    std::vector<id<MTLBuffer>> handles(binding.arrayCount);
    std::vector<NSUInteger> nsOffsets(binding.arrayCount);
    for (size_t i = 0; i < binding.arrayCount; i++)
    {
        handles[i] = metalBuffers[i]->handle;
        nsOffsets[i] = binding.resource.offsets[i];
    }

    if ((binding.stage & EShaderStage::Vertex) != EShaderStage::None)
    {
        const uint32 stageBinding = resolveStageBinding(binding, EShaderStage::Vertex);
        [curRenderEncoder setVertexBuffers:handles.data() offsets:nsOffsets.data() withRange:NSMakeRange(stageBinding, binding.arrayCount)];
    }
    if ((binding.stage & EShaderStage::Fragment) != EShaderStage::None)
    {
        const uint32 stageBinding = resolveStageBinding(binding, EShaderStage::Fragment);
        [curRenderEncoder setFragmentBuffers:handles.data() offsets:nsOffsets.data() withRange:NSMakeRange(stageBinding, binding.arrayCount)];
    }
}

void MetalCommandBuffer::bindTextures(const ResourceBinding& binding)
{
    MetalTexture* const* mtlTextures = reinterpret_cast<MetalTexture* const*>(binding.resource.textures.data());
    std::vector<id<MTLTexture>> handles(binding.arrayCount);
    for (size_t i = 0; i < binding.arrayCount; i++)
    {
        handles[i] = mtlTextures[i]->handle;
    }

    if ((binding.stage & EShaderStage::Vertex) != EShaderStage::None)
    {
        const uint32 stageBinding = resolveStageBinding(binding, EShaderStage::Vertex);
        [curRenderEncoder setVertexTextures:handles.data() withRange:NSMakeRange(stageBinding, binding.arrayCount)];
    }
    if ((binding.stage & EShaderStage::Fragment) != EShaderStage::None)
    {
        const uint32 stageBinding = resolveStageBinding(binding, EShaderStage::Fragment);
        [curRenderEncoder setFragmentTextures:handles.data() withRange:NSMakeRange(stageBinding, binding.arrayCount)];
    }
}

void MetalCommandBuffer::bindSamplers(const ResourceBinding& binding)
{
    MetalSampler* const* mtlSamplers = reinterpret_cast<MetalSampler* const*>(binding.resource.samplers.data());
    std::vector<id<MTLSamplerState>> handles(binding.arrayCount);
    for (size_t i = 0; i < binding.arrayCount; i++)
    {
        handles[i] = mtlSamplers[i]->handle;
    }

    if ((binding.stage & EShaderStage::Vertex) != EShaderStage::None)
    {
        const uint32 stageBinding = resolveStageBinding(binding, EShaderStage::Vertex);
        [curRenderEncoder setVertexSamplerStates:handles.data() withRange:NSMakeRange(stageBinding, binding.arrayCount)];
    }
    if ((binding.stage & EShaderStage::Fragment) != EShaderStage::None)
    {
        const uint32 stageBinding = resolveStageBinding(binding, EShaderStage::Fragment);
        [curRenderEncoder setFragmentSamplerStates:handles.data() withRange:NSMakeRange(stageBinding, binding.arrayCount)];
    }
}
HS_NS_END
