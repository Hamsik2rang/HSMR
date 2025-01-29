//
//  ViewController.m
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//

#import "ViewController.h"
#import "Renderer.h"

@implementation HSViewController
{
    MTKView* _view;
    HSRenderer* _renderer;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Do any additional setup after loading the view.
    _view = (MTKView*)self.view;

    _view.device = MTLCreateSystemDefaultDevice();

    NSAssert(_view.device, @"Metal is not supported on this device");
    
    
    _view.device = MTLCreateSystemDefaultDevice();
    
    _view.clearColor = MTLClearColorMake(0.5, 0.5, 0.5, 1.0);
    
    _renderer = new HSRenderer();
    if (_renderer->Init(_view))
    {
        NSAssert(_renderer, @"Renderer failed initialization");
    }
//    _view.delegate = _renderer->GetDelegate();
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

@end
