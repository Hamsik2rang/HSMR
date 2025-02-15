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

#include "Engine/Renderer/ResourceHandle.h"

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
    
    void Configure(Framebuffer* renderTarget) override;
    
    void Execute(void* cmdEncoder, RenderPass* renderPass) override;
    
    void OnAfterRendering() override;
    
private:
    RHI_RESOURCE_DEFINE(ForwardOpaquePass)
    
    void createResourceHandles();
    void createPipelineHandles(RenderPass* renderPass);
    
    Framebuffer* _currentFramebuffer;

};

HS_NS_END

#endif
