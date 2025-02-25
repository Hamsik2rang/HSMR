#include "Engine/RHI/Metal/CommandHandleMetal.h"

#include "Engine/RHI/Metal/RHIContextMetal.h"
#include "Engine/RHI/Metal/ResourceHandleMetal.h"
#include "Engine/RHI/Metal/RenderHandleMetal.h"

#include "Engine/Core/Log.h"

#include <vector>

HS_NS_BEGIN

CommandBufferMetal::CommandBufferMetal(id<MTLDevice> device, id<MTLCommandQueue> commandQueue)
    : device(device)
    , cmdQueue(commandQueue)
{
}

CommandBufferMetal::~CommandBufferMetal()
{
}

void CommandBufferMetal::Begin()
{
    HS_ASSERT(!_isBegan, "CommandBuffer is already began");

    handle = [cmdQueue commandBufferWithUnretainedReferences];

    _isBegan           = true;
    _isRenderPassBegan = false;
}

void CommandBufferMetal::End()
{
    HS_ASSERT(_isBegan, "Commandbuffer isn't began yet");

    Reset();
}

void CommandBufferMetal::Reset()
{
    @autoreleasepool
    {
        if (nil != curRenderEncoder)
        {
            [curRenderEncoder endEncoding];
            curRenderEncoder = nil;
        }
    }
    _isBegan = false;
}

void CommandBufferMetal::BeginRenderPass(RenderPass* renderPass, Framebuffer* framebuffer)
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");
    HS_CHECK(renderPass, "RenderPass is null");
    @autoreleasepool
    {

        if (renderPass->info.isSwapchainRenderPass)
        {
            //        HS_ASSERT(framebuffer->info.isSwapchainFramebuffer, "Swapchain RenderPass, but Framebuffer isn't");
            curRenderPassDesc = static_cast<RenderPassMetal*>(renderPass)->handle;
        }
        else
        {
            HS_CHECK(framebuffer, "Framebuffer is null");
            HS_CHECK(renderPass == framebuffer->info.renderPass, "RenderPass is not same with Framebuffer's RenderPass");
            curRenderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];

            bool useDepthStencil = renderPass->info.useDepthStencilAttachment;

            size_t i = 0;
            for (; i < renderPass->info.colorAttachmentCount; i++)
            {
                const Attachment& curAttachment = renderPass->info.colorAttachments[i];

                curRenderPassDesc.colorAttachments[i].texture     = static_cast<TextureMetal*>(framebuffer->info.colorBuffers[i])->handle;
                curRenderPassDesc.colorAttachments[i].loadAction  = hs_rhi_to_load_action(curAttachment.loadAction);
                curRenderPassDesc.colorAttachments[i].storeAction = hs_rhi_to_store_action(curAttachment.storeAction);
                curRenderPassDesc.colorAttachments[i].clearColor  = hs_rhi_to_clear_color(curAttachment.clearValue.color);
            }

            if (useDepthStencil)
            {
                const Attachment curAttachment                = renderPass->info.depthStencilAttachment;
                curRenderPassDesc.depthAttachment.texture     = static_cast<TextureMetal*>(framebuffer->info.depthStencilBuffer)->handle;
                curRenderPassDesc.depthAttachment.loadAction  = hs_rhi_to_load_action(curAttachment.loadAction);
                curRenderPassDesc.depthAttachment.storeAction = hs_rhi_to_store_action(curAttachment.storeAction);
                curRenderPassDesc.depthAttachment.clearDepth  = static_cast<double>(curAttachment.clearValue.depth);
            }
        }

        if (nil != curRenderEncoder)
        {
            [curRenderEncoder endEncoding];
        }

        curRenderEncoder          = [handle renderCommandEncoderWithDescriptor:curRenderPassDesc];
        curBindRenderPass         = static_cast<RenderPassMetal*>(renderPass);
        curBindRenderPass->handle = curRenderPassDesc;
        curBindFramebuffer        = static_cast<FramebufferMetal*>(framebuffer);
        _isRenderPassBegan        = true;
    }
}

void CommandBufferMetal::BindPipeline(GraphicsPipeline* pipeline)
{
    HS_CHECK(_isBegan, "CommandBuffer isn't began yet");
    HS_CHECK(_isRenderPassBegan, "RenderPass isn't began yet");
    HS_CHECK(pipeline, "Pipeline is null");
    curBindPipeline = static_cast<GraphicsPipelineMetal*>(pipeline);

    @autoreleasepool
    {
        [curRenderEncoder setRenderPipelineState:curBindPipeline->pipelineState];
        if (curBindRenderPass->info.useDepthStencilAttachment)
        {
            [curRenderEncoder setDepthStencilState:curBindPipeline->depthStencilState];
        }
        [curRenderEncoder setFrontFacingWinding:hs_rhi_to_front_face(pipeline->info.rasterizerDesc.frontFace)];
        [curRenderEncoder setCullMode:hs_rhi_to_cull_mode(pipeline->info.rasterizerDesc.cullMode)];
        [curRenderEncoder setTriangleFillMode:hs_rhi_to_polygon_mode(pipeline->info.rasterizerDesc.polygonMode)];
    }
    curBindPipeline = static_cast<GraphicsPipelineMetal*>(pipeline);
}

void CommandBufferMetal::BindResourceSet(ResourceSet* rSet)
{
    @autoreleasepool
    {
        for (size_t i = 0; i < rSet->layouts.size(); i++)
        {
            const auto& bindings = rSet->layouts[i]->bindings;
            for (size_t j = 0; j < bindings.size(); j++)
            {
                const ResourceBinding& rb = bindings[j];

                switch (rb.type)
                {
                    case EResourceType::UNIFORM_BUFFER:
                    {
                        bindBuffers(rb.stage, rb.binding, rb.resource.buffers.data(), rb.resource.offsets.data(), rb.arrayCount);
                    }
                    break;
                    case EResourceType::COMBINED_IMAGE_SAMPLER:
                    case EResourceType::SAMPLED_IMAGE:
                    {
                        bindTextures(rb.stage, rb.binding, rb.resource.textures.data(), rb.arrayCount);
                    }
                    break;
                    case EResourceType::SAMPLER:
                    {
                        bindSamplers(rb.stage, rb.binding, rb.resource.samplers.data(), rb.arrayCount);
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
}

void CommandBufferMetal::SetViewport(const Viewport& viewport)
{
    @autoreleasepool
    {
        [curRenderEncoder setViewport:hs_rhi_to_viewport(viewport)];
    }
}

void CommandBufferMetal::SetScissor(const uint32 x, const uint32 y, const uint32 width, const uint32 height)
{
    @autoreleasepool
    {
        MTLScissorRect rect = {x, y, width, height};
        [curRenderEncoder setScissorRect:rect];
    }
}

void CommandBufferMetal::BindIndexBuffer(Buffer* indexBuffer)
{
    @autoreleasepool
    {
        curBindIndexBuffer = static_cast<BufferMetal*>(indexBuffer);
    }
}

void CommandBufferMetal::BindVertexBuffers(const Buffer* const* vertexBuffers, const uint32* offsets, const uint8 bufferCount)
{
    @autoreleasepool
    {
        for (uint8 i = 0; i < bufferCount; i++)
        {
            auto vertexBuffer = static_cast<const BufferMetal*>(vertexBuffers[i]);

            [curRenderEncoder setVertexBuffer:vertexBuffer->handle offset:offsets[i] atIndex:i];
        }
    }
}

void CommandBufferMetal::DrawArrays(const uint32 firstVertex, const uint32 vertexCount, const uint32 instanceCount)
{
    @autoreleasepool
    {
        MTLPrimitiveType primType = hs_rhi_to_primitive_topology(curBindPipeline->info.inputAssemblyDesc.primitiveTopology);

        [curRenderEncoder drawPrimitives:primType
                             vertexStart:firstVertex
                             vertexCount:vertexCount
                           instanceCount:instanceCount
                            baseInstance:0];
    }
}
void CommandBufferMetal::DrawIndexed(const uint32 firstIndex, const uint32 indexCount, const uint32 instanceCount, const uint32 vertexOffset)
{
    @autoreleasepool
    {
        MTLPrimitiveType primType = hs_rhi_to_primitive_topology(curBindPipeline->info.inputAssemblyDesc.primitiveTopology);

        [curRenderEncoder drawIndexedPrimitives:primType
                                     indexCount:indexCount
                                      indexType:MTLIndexTypeUInt32
                                    indexBuffer:curBindIndexBuffer->handle
                              indexBufferOffset:firstIndex
                                  instanceCount:instanceCount
                                     baseVertex:vertexOffset
                                   baseInstance:0];
    }
}

void CommandBufferMetal::EndRenderPass()
{
    curRenderPassDesc  = nil;
    curBindRenderPass  = nullptr;
    curBindFramebuffer = nullptr;
    curBindPipeline    = nullptr;

    _isRenderPassBegan = false;
}

void CommandBufferMetal::CopyTexture(Texture* srcTexture, Texture* dstTexture)
{
}
void CommandBufferMetal::UpdateBuffer(Buffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize)
{
}

void CommandBufferMetal::PushDebugMark(const char* label, float* color)
{
    HS_ASSERT(_isBegan, "CommandBuffer isn't began yet");
    @autoreleasepool
    {
        NSString* labelStr = [NSString stringWithCString:label encoding:NSUTF8StringEncoding];
        [curRenderEncoder pushDebugGroup:labelStr];
    }
}

void CommandBufferMetal::PopDebugMark()
{
    [curRenderEncoder popDebugGroup];
}

void CommandBufferMetal::bindBuffers(EShaderStage stage, uint8 binding, Buffer* const* buffers, const uint32* offsets, uint8 arrayCount)
{
    @autoreleasepool
    {
        BufferMetal* const*        bufferMetals = reinterpret_cast<BufferMetal* const*>(buffers);
        std::vector<id<MTLBuffer>> handles(arrayCount);
        std::vector<NSUInteger>    nsOffsets(arrayCount);
        for (size_t i = 0; i < arrayCount; i++)
        {
            nsOffsets[i] = offsets[i];
        }

        switch (stage)
        {
            case EShaderStage::VERTEX:
            {
                [curRenderEncoder setVertexBuffers:handles.data() offsets:nsOffsets.data() withRange:NSMakeRange(binding, arrayCount)];
            }
            break;
            case EShaderStage::FRAGMENT:
            {
                [curRenderEncoder setFragmentBuffers:handles.data() offsets:nsOffsets.data() withRange:NSMakeRange(binding, arrayCount)];
            }
            break;
            default:
            {
                HS_LOG(crash, "Not Implemented ResourceType");
            }
            break;
        }
    }
}

void CommandBufferMetal::bindTextures(EShaderStage stage, uint8 binding, Texture* const* textures, uint8 arrayCount)
{
    @autoreleasepool
    {
        TextureMetal* const*        textureMetals = reinterpret_cast<TextureMetal* const*>(textures);
        std::vector<id<MTLTexture>> handles(arrayCount);

        switch (stage)
        {
            case EShaderStage::VERTEX:
            {
                [curRenderEncoder setVertexTextures:handles.data() withRange:NSMakeRange(binding, arrayCount)];
            }
            case EShaderStage::FRAGMENT:
            {
                [curRenderEncoder setFragmentTextures:handles.data() withRange:NSMakeRange(binding, arrayCount)];
            }
            default:
            {
                HS_LOG(crash, "Not Implemented ResourceType");
            }
            break;
        }
    }
}

void CommandBufferMetal::bindSamplers(EShaderStage stage, uint8 binding, Sampler* const* samplers, uint8 arrayCount)
{
    @autoreleasepool
    {
        SamplerMetal* const*             samplerMetals = reinterpret_cast<SamplerMetal* const*>(samplers);
        std::vector<id<MTLSamplerState>> handles(arrayCount);

        switch (stage)
        {
            case EShaderStage::VERTEX:
            {
                [curRenderEncoder setVertexSamplerStates:handles.data() withRange:NSMakeRange(binding, arrayCount)];
            }
            break;
            case EShaderStage::FRAGMENT:
            {
                [curRenderEncoder setFragmentSamplerStates:handles.data() withRange:NSMakeRange(binding, arrayCount)];
            }
            break;
            default:
            {
                HS_LOG(crash, "Not Implemented ResourceType");
            }
            break;
        }
    }
}
HS_NS_END
