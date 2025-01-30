//
//  Renderer.mm
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//

#include "Renderer.h"
#include "ViewController.h"
#include "RendererPass.h"
#include "ForwardTrianglePass.h"

HSRenderer::HSRenderer(id<MTLDevice> device)
    : _device(device)
{
}

HSRenderer::~HSRenderer()
{
}

bool HSRenderer::Init(HSView* view)
{
    _view = view;
    _commandQueue = [_device newCommandQueue];
    if (_commandQueue == nil)
    {
        return false;
    }

    for (size_t i = 0; i < MAX_SUBMIT_INDEX; i++)
    {
        MTLTextureDescriptor* textureDesc = [MTLTextureDescriptor alloc];
        textureDesc.width = view.drawableSize.width;
        textureDesc.height = view.drawableSize.height;
        textureDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        textureDesc.pixelFormat = view.colorPixelFormat;
        textureDesc.mipmapLevelCount = 1;
        textureDesc.textureType = MTLTextureType2D;
        textureDesc.depth = 1;
        textureDesc.arrayLength = 1;
        textureDesc.swizzle = MTLTextureSwizzleChannelsDefault;
        textureDesc.sampleCount = 1;
        
        _renderTarget[i] = [_device newTextureWithDescriptor:textureDesc];
    }
    
    return true;
}

void HSRenderer::NextFrame()
{
    _submitIndex = (_submitIndex + 1) % MAX_SUBMIT_INDEX;
}

void HSRenderer::Render()
{
    _commandBuffer = [_commandQueue commandBuffer];

    for (auto* pass : _rendererPasses)
    {
        pass->OnBeforeRendering(_submitIndex);
    }

    for (auto* pass : _rendererPasses)
    {
        pass->Configure(_renderTarget[_submitIndex]);

        if (pass->IsExecutable())
        {
            pass->Execute(_commandBuffer);
        }
    }

    for (auto* pass : _rendererPasses)
    {
        pass->OnAfterRendering();
    }
}

void HSRenderer::Present(id<MTLDrawable> currentDrawable)
{
    [_commandBuffer presentDrawable:currentDrawable];

    [_commandBuffer commit];
}

HSView* HSRenderer::GetView()
{
    return _view;
}
