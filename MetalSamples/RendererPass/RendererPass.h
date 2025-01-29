//
//  RendererPass.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/30/25.
//

#import <Metal/Metal.h>

class HSRendererPass
{
public:
    HSRendererPass();
    ~HSRendererPass();
    
    virtual void OnInit() = 0;
    
    virtual void OnBeforeRendering() = 0;
    
    virtual void Configure() = 0;
    
    virtual void Execute() = 0;
    
    virtual void OnAfterRendering() = 0;
    
    bool IsExecutable() { return _isExecutable; }
    
protected:
    bool _isExecutable = true;
};
