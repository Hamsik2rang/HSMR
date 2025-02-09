#include "Engine/Renderer/RenderPass.h"

#include "Engine/Renderer/RHIUtility.h"
#import <Metal/Metal.h>

HS_NS_BEGIN

RenderPass::RenderPass(RenderPassInfo info)
    : info(info)
{
    MTLRenderPassDescriptor* rpDesc = [MTLRenderPassDescriptor new];
    handle = hs_rhi_from_render_pass_desc(rpDesc);


//    for (size_t i = 0; i < info.colorAttachmentCount; i++)
//    {
//        rpDesc.colorAttachments[i].clearColor  = hs_rhi_to_clear_color(info.colorAttachments[0].clearColor);
//        rpDesc.colorAttachments[i].loadAction  = hs_rhi_to_load_action(info.colorAttachments[0].loadAction);
//        rpDesc.colorAttachments[i].storeAction = hs_rhi_to_store_action(info.colorAttachments[0].storeAction);
//    }

    if (info.useDepthStencilAttachment)
    {
        //...
    }
}

RenderPass::~RenderPass()
{
}

HS_NS_END
