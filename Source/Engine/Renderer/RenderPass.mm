#include "Engine/Renderer/RenderPass.h"

#include "Engine/Renderer/RHIUtility.h"
#import <Metal/Metal.h>

HS_NS_BEGIN

RenderPass::RenderPass(RenderPassInfo info)
    : info(info)
{
    MTLRenderPassDescriptor* rpDesc = [MTLRenderPassDescriptor new];
    handle                          = hs_rhi_from_render_pass_desc(rpDesc);

    size_t i = 0;
    for (i = 0; i < info.colorAttachmentCount; i++)
    {
        //        rpDesc.colorAttachments[i].
    }
    if (info.useDepthStencilAttachment)
    {
        //...
    }
}

RenderPass::~RenderPass()
{
}

HS_NS_END
