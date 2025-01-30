//
//  ViewController.m
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//

#import "ViewController.h"
#include "Renderer.h"
#include "BasicClearPass.h"

@implementation HSView

// signal that we want our window be the first responder of user inputs
- (BOOL)acceptsFirstResponder
{
    return YES;
}

#if !(TARGET_OS_IPHONE)
- (void)awakeFromNib
{
    // on osx, create a tracking area to keep track of the mouse movements and events
    NSTrackingAreaOptions options = (NSTrackingActiveAlways | NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);

    NSTrackingArea* area = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                        options:options
                                                          owner:self
                                                       userInfo:nil];
    [self addTrackingArea:area];
}

- (BOOL)acceptsFirstMouse:(NSEvent*)event
{
    return YES;
}

#endif
@end

@implementation HSViewController
{
    HSView* _view;
    HSRenderer* _renderer;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    _view = (HSView*)self.view;

    _view.device = MTLCreateSystemDefaultDevice();

    NSAssert(_view.device, @"Metal is not supported on this device");

    _view.device = MTLCreateSystemDefaultDevice();

    [self mtkView:_view drawableSizeWillChange:_view.drawableSize];

    _view.delegate = self;

    _view.clearColor = MTLClearColorMake(0.0, 0.8, 1.0, 1.0);
    
    _renderer = new HSRenderer(_view.device);
    if (_renderer->Init(_view))
    {
        NSAssert(_renderer, @"Renderer failed initialization");
    }
    //    _view.delegate = _renderer->GetDelegate();
    _renderer->AddPass(new HSBasicClearPass("BasicClearPass", _renderer, 0));
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (void)drawInMTKView:(nonnull MTKView*)view
{
    _renderer->NextFrame();

    _renderer->Render();

    id<MTLDrawable> drawable = view.currentDrawable;

    _renderer->Present(drawable);
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

@end
