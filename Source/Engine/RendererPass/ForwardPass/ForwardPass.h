//
//  ForwardPass.h
//  HSMR
//
//  Created by Yongsik Im on 2/4/25.
//
#ifndef __HS_FORWARD_PASS_H__
#define __HS_FORWARD_PASS_H__

#include "Precompile.h"
#include "Engine/RendererPass/RendererPass.h"

HS_NS_BEGIN

class ForwardPass : public RendererPass
{
public:
    ForwardPass(const char* name, Renderer* renderer, uint32 renderingOrder)
    : RendererPass(name, renderer, renderingOrder)
    {}
    
private:
    
    
};

HS_NS_END

#endif
