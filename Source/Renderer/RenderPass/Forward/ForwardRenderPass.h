//
//  ForwardPass.h
//  HSMR
//
//  Created by Yongsik Im on 2/4/25.
//
#ifndef __HS_FORWARD_PASS_H__
#define __HS_FORWARD_PASS_H__

#include "Precompile.h"
#include "Core/RendererPass/RendererPass.h"

HS_NS_BEGIN

class ForwardPass : public RenderPass
{
public:
    ForwardPass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder);
    ~ForwardPass() override {}

protected:
    
};

HS_NS_END

#endif
