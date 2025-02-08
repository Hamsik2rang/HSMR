#include "Engine/Renderer/RenderDefinition.h"

#include "Engine/Core/Log.h"
#include "Engine/Renderer/Renderer.h"

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

HS_NS_BEGIN

#define HTEXTURE(x) ((__bridge id<MTLTexture>)(x))
#define HBUFFER(x) ((__bridge id<MTLBufer>)(x))


HS_NS_END
