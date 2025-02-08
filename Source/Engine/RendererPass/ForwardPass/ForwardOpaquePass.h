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

#include "Engine/REnderer/ResourceHandle.h"

HS_NS_BEGIN

class RenderPass;
class Framebuffer;
class Pipeline;

class ForwardOpaquePass : public ForwardPass
{
public:
    ForwardOpaquePass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder);
    ~ForwardOpaquePass() override;
    
    void OnBeforeRendering(uint32_t submitIndex) override;
    
    void Configure(RenderTexture* renderTarget) override;
    
    void Execute(RenderCommandEncoder* renderEncoder) override;
    
    void OnAfterRendering() override;
    
private:
   
    void createResourceHandles();
    
    RenderTexture* _currentRenderTexture;

};

HS_NS_END

#endif
