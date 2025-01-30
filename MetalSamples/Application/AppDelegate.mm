//
//  AppDelegate.m
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//

#import "AppDelegate.h"

@interface HSAppDelegate ()


@end

@implementation HSAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    NSLog(@"Terminate!");
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication*) sender
{
    return YES;
}

@end
