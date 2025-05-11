//
//  PlatformCommon.h
//  Engine
//
//  Created by Yongsik Im on 5/6/25.
//
#ifndef __HS_PLATFORM_COMMON_H__
#define __HS_PLATFORM_COMMON_H__

#include "Precompile.h"

#if defined(__APPLE__)
#include "Engine/Platform/OSX/PlatformOSX.h"
#else
#include "Engine/Platform/Windows/PlatformWindows.h"
#endif

HS_NS_BEGIN



HS_NS_END

#endif /*__HS_PLATFORM_COMMON_H__*/
