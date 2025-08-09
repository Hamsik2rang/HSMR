//
//  PlatformApplication.h
//  Engine
//
//  Created by Yongsik Im on 2025.5.16
//
#ifndef __HS_PLATFORM_APPLICATION_H__
#define __HS_PLATFORM_APPLICATION_H__

#include "Precompile.h"

namespace HS { class Application; }

HS_NS_BEGIN


void hs_platform_init(Application* hsApp);
void hs_platform_shutdown(Application* hsApp);

HS_NS_END

#endif /*__HS_PLATFORM_APPLICATION_H__*/
