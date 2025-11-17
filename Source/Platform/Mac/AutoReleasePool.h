//
//  AutoReleasePool.h
//  Engine
//
//  Created by Yongsik Im on 5/24/2025
//
#if defined(__APPLE__)

#ifndef __HS_AUTO_RELEASE_POOL_H__
#define __HS_AUTO_RELEASE_POOL_H__

#include "Precompile.h"


HS_NS_BEGIN

struct HS_API AutoReleasePool
{
    void* p;
    AutoReleasePool();
    ~AutoReleasePool();
};


HS_NS_END


#endif /*__HS_AUTO_RELEASE_POOL_H__*/

#endif /*__APPLE__*/
