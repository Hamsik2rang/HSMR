//
//  MacWindow.h
//  HAL
//
//  Created by Yongsik Im on 8/9/2025
//
#ifndef __HS_MAC_WINDOW_H__
#define __HS_MAC_WINDOW_H__

#include "Precompile.h"

#import <Cocoa/Cocoa.h>


@interface HSViewController : NSViewController<NSWindowDelegate>

@property (nonatomic, strong) NSWindow* window;

- (instancetype)initWithWindow:(NSWindow*)window;
- (CGSize)getBackingViewSize;

@end

HS_NS_BEGIN

HS_NS_END

#endif /*__HS_PLATFORM_OSX_H__*/
