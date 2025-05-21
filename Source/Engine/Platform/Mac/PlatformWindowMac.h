//
//  PlatformWindowMac.h
//  Engine
//
//  Created by Yongsik Im on 2025.5.16
//
#ifndef __HS_PLATFORM_OSX_H__
#define __HS_PLATFORM_OSX_H__

#include "Precompile.h"

#include "Engine/Platform/PlatformWindow.h"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

@interface HSViewController : NSViewController <NSWindowDelegate>

@property (nonatomic, strong) NSWindow* window;

@end

HS_NS_BEGIN


HS_NS_END

#endif /*__HS_PLATFORM_OSX_H__*/
