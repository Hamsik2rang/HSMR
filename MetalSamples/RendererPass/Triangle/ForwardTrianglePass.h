//
//  ForwardTrianglePass.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __FORWARD_TRIANGLE_PASS_H__
#define __FORWARD_TRIANGLE_PASS_H__

#include "RendererPass.h"

class HSForwardTrianglePass : public HSRendererPass
{
public:
    void OnInit() override;
    
    void OnBeforeRendering(uint32_t submitIndex) override;
    
    void Configure(id<MTLTexture> renderTarget) override;
    
    void Execute(id<MTLCommandBuffer> cmdBuffer) override;
    
    void OnAfterRendering() override;
    
private:
    
};


#endif
