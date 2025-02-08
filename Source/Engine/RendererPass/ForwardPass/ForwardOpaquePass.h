//
//  BasicClearPass.h
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//
#ifndef __BASIC_CLEAR_PASS_H__
#define __BASIC_CLEAR_PASS_H__

#include "Precompile.h"
#include "ForwardPass.h"

HS_NS_BEGIN

class ForwardOpaquePass : public ForwardPass
{
public:
    ForwardOpaquePass(const char* name, Renderer* renderer, uint32_t renderingOrder);
    ~ForwardOpaquePass() override;
    
    void Initialize() override;
    
    void Finalize() override;
    
    void OnBeforeRendering(uint32_t submitIndex) override;
    
    void Configure(RenderTexture* renderTarget) override;
    
    void Execute(RenderCommandEncoder* renderEncoder) override;
    
    void OnAfterRendering() override;
    
    
private:
    
    RenderTexture* _currentRenderTarget;

};

HS_NS_END

#endif
