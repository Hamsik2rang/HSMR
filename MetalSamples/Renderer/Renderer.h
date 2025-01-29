//
//  Renderer.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//

#import <MetalKit/MetalKit.h>
#import <Metal/Metal.h>

#include <vector>

// A platform-independent renderer class.
@interface HSInternalDelegate : NSObject<MTKViewDelegate>

@end


class HSRenderer
{
public:
    HSRenderer();
    ~HSRenderer();

    bool Init(MTKView* mtkView);

    void Render();
    
    MTKView* GetMTKView() { return _view; }
    HSInternalDelegate* GetDelegate() { return _delegate; }

private:
    MTKView* _Nullable _view = nullptr;
    HSInternalDelegate* _delegate;
    
    
};
