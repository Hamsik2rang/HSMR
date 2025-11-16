//
//  StringUtility.h
//  Core
//
//  Created by Yongsik Im on 8/10/2025.
//
#ifndef __HS_STRING_UTILITY_H__
#define __HS_STRING_UTILITY_H__

#include "Precompile.h"
#include <cstdarg>

HS_NS_BEGIN

namespace StringUtil
{

std::string HS_API Format(const char* fmt, ...);

}

HS_NS_END

#endif
