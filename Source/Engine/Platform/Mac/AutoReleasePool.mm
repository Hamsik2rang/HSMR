#include "Engine/Platform/Mac/AutoReleasePool.h"

#include <Foundation/Foundation.h>

HS_NS_BEGIN

AutoReleasePool::AutoReleasePool()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    p = (__bridge void*)pool;
}

AutoReleasePool::~AutoReleasePool()
{
    NSAutoreleasePool* pool = (__bridge NSAutoreleasePool*)p;
    [pool release];
}

HS_NS_END
