//
//  RenderPass.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//

#ifndef __HS_RENDER_PASS_H__
#define __HS_RENDER_PASS_H__


#include "Precompile.h"

#include "Engine/Renderer/RenderDefinition.h"

HS_NS_BEGIN

class RenderPass : public RHIHandle
{
public:
    RenderPass(RenderPassInfo info);
    ~RenderPass();
    
    RenderPassInfo info;
};

HS_NS_END

#endif /* __HS_RENDER_PASS_H__ */
