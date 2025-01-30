//
//  ViewController.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//
#ifndef __VIEW_CONTROLLER_H__
#define __VIEW_CONTROLLER_H__

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

@interface HSView : MTKView

- (BOOL)acceptsFirstResponder;
#if !TARGET_OS_IPHONE
- (BOOL)acceptsFirstMouse:(NSEvent *)event;
#endif

@end

@interface HSViewController : NSViewController<MTKViewDelegate>


@end

#endif
