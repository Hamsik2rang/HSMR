//
//  FrameBuffer.h
//  Engine
//
//  Created by Yongsik Im on 2/8/25.
//

#ifndef __HS_FRAMEBUFFER_H__
#define __HS_FRAMEBUFFER_H__

#include "Precompile.h"

#include "Engine/RHI/RenderDefinition.h"

#include <vector>

HS_NS_BEGIN

class Framebuffer : public RHIHandle
{
public:
    Framebuffer(FramebufferInfo info);
    ~Framebuffer() override;
    
    void Resize(uint32 width, uint32 height);
    
    FramebufferInfo info;

    RenderPass* GetRenderPass() { return _renderPass;}
private:
    //...
    RenderPass* _renderPass;
};

HS_NS_END

#endif /* __HS_FRAME_BUFFER_H__ */
