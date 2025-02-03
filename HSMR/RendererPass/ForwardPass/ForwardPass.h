//
//  ForwardPass.h
//  HSMR
//
//  Created by Yongsik Im on 2/4/25.
//
#ifndef __HS_FORWARD_PASS_H__
#define __HS_FORWARD_PASS_H__

#include "RendererPass.h"

class HSForwardPass : public HSRendererPass
{
public:
    HSForwardPass(const char* name, HSRenderer* renderer, uint32 renderingOrder)
    : HSRendererPass(name, renderer, renderingOrder)
    {}
    
private:
    
    
};


#endif
