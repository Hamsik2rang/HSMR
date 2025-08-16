//
//  ForwardRenderPass.h
//  HSMR
//
//  Created by Yongsik Im on 2/4/25.
//
#ifndef __HS_FORWARD_PASS_H__
#define __HS_FORWARD_PASS_H__

#include "Precompile.h"
#include "Renderer/RenderPass/RenderPass.h"

HS_NS_BEGIN

class ForwardRenderPass : public RenderPass
{
public:
    ForwardRenderPass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder);
    ~ForwardRenderPass() override {}

protected:
    
};

HS_NS_END

#endif
