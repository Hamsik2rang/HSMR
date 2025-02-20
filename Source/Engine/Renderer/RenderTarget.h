//
//  RenderTarget.h
//  HSMR
//
//  Created by Yongsik Im on 11/2/25.
//
#ifndef __HS_RENDER_TARGET_H__
#define __HS_RENDER_TARGET_H__

#include "Precompile.h"

#include "Engine/RHI/ResourceHandle.h"

HS_NS_BEGIN

class RenderTarget
{
public:
    RenderTarget();
    ~RenderTarget();

    uint32 GetWidth() const { return _width; }
    uint32 GetHeight() const { return _height; }

    Texture* GetColorTexture(uint32 index) { return _colorTextures[index]; }
    Texture* GetDepthStencilTexture() const { return _depthStencilTexture; }
    
    const std::vector<Texture*>& GetColorTextures() { return _colorTextures; }

private:
    uint32 _width;
    uint32 _height;

    std::vector<Texture*> _colorTextures;
    Texture*              _depthStencilTexture;
};

HS_NS_END

#endif /* __HS_RENDER_TARGET_H__ */
