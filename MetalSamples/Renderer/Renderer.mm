//
//  Renderer.mm
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//

#include "Renderer.h"

@interface HSInternalDelegate ()

@end

@implementation HSInternalDelegate
{
    HSRenderer* _renderer;
}

- (nonnull instancetype)initWithRenderer:(nonnull HSRenderer*)renderer
{
    self = [super init];
    if (self)
    {
        _renderer = renderer;
        MTKView* view = _renderer->GetMTKView();
        [self mtkView:view drawableSizeWillChange:view.drawableSize];
        
        view.delegate = self;
    }

    return self;
}

- (void)drawInMTKView:(nonnull MTKView*)view
{
    _renderer->Render();
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

@end

HSRenderer::HSRenderer()
{
}

HSRenderer::~HSRenderer()
{
}

bool HSRenderer::Init(MTKView* mtkView)
{
    _delegate = [[HSInternalDelegate alloc] initWithRenderer:this];

    return true;
}

void HSRenderer::Render()
{
    
}
