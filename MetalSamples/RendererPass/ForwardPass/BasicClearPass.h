//
//  BasicClearPass.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __BASIC_CLEAR_PASS_H__
#define __BASIC_CLEAR_PASS_H__

#include "RendererPass.h"

class HSBasicClearPass : public HSRendererPass
{
public:
    HSBasicClearPass(const char* name, HSRenderer* renderer, uint32_t renderingOrder);
    ~HSBasicClearPass() override;
    
    void Initialize() override;
    
    void Finalize() override;
    
    void OnBeforeRendering(uint32_t submitIndex) override;
    
    void Configure(id<MTLTexture> renderTarget) override;
    
    void Execute(id<MTLRenderCommandEncoder> renderEncoder) override;
    
    void OnAfterRendering() override;
    
    
private:
};







#endif
