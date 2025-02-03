//
//  DebugPass.h
//  MetalSamples
//
//  Created by Yongsik Im on 2/2/25.
//
#ifndef __HS_DEBUG_PASS_H__
#define __HS_DEBUG_PASS_H__

#include "RendererPass.h"

class HSDebugPass : public HSRendererPass
{
public:
    void Initialize() override;
    
    void Finalize() override;
    
    void OnBeforeRendering(uint32_t submitIndex) override;
    
    void Configure(id<MTLTexture> renderTarget) override;
    
    void Execute(id<MTLRenderCommandEncoder> renderEncoder) override;
    
    void OnAfterRendering() override;
    
private:
    
};



#endif
