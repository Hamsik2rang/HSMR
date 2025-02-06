//
//  DebugPass.h
//  HSMR
//
//  Created by Yongsik Im on 2/2/25.
//
#ifndef __HS_DEBUG_PASS_H__
#define __HS_DEBUG_PASS_H__

#include "Precompile.h"
#include "RendererPass.h"

HS_NS_BEGIN

class DebugPass : public RendererPass
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

HS_NS_END

#endif
